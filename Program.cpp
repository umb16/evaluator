//
//  Program.cpp
//  Evaluator
//
//  Created by Damien Quartz on 10/9/17
//
//

// required to get M_PI on windows
#define _USE_MATH_DEFINES

#include "Program.h"
#include <ctype.h>
#include <deque>
#include <math.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
// COMPILATION
//////////////////////////////////////////////////////////////////////////
#pragma region Compilation

const char * Program::GetErrorString(Program::CompileError error)
{
	switch (error)
	{
	case Program::CE_NONE:
		return "None";
	case Program::CE_MISSING_PAREN:
		return "Mismatched parens";
	case Program::CE_UNEXPECTED_CHAR:
		return "Unexpected character";
	case Program::CE_ILLEGAL_ASSIGNMENT:
		return "Left side of = must be assignable (a variable or address)";
	case Program::CE_MISSING_BRACKET:
		return "Missing ]";
	case Program::CE_ILLEGAL_STATEMENT_TERMINATION:
		return "Illegal statement termination.\nSemi-colon may not appear within parens or ternary operators.";
	default:
		return "Unknown";
	}
}

// used during compilation to keep track of things
struct CompilationState
{
	const Program::Char* const source;
	int parsePos;
	int parenCount;
	int bracketCount;
	int parseDepth;
	Program::CompileError error;
	std::vector<Program::Op> ops;

	CompilationState(const Program::Char* inSource)
		: source(inSource)
		, parsePos(0)
		, parenCount(0)
		, bracketCount(0)
		, parseDepth(0)
		, error(Program::CE_NONE)
	{

	}

	// some helpers
	Program::Char operator*() const { return source[parsePos]; }
	void Push(Program::Op::Code code, Program::Value value = 0) { ops.push_back(Program::Op(code, value)); }
	void SkipWhitespace()
	{
		while (isspace(source[parsePos]))
		{
			++parsePos;
		}
	}
};

static int Parse(CompilationState& state);

static int ParseAtom(CompilationState& state)
{
	// Skip spaces
	state.SkipWhitespace();

	std::stack<Program::Op::Code> unaryOps;

	Program::Char op = *state;
	while (op == '-' || op == '+' || op == '$' || op == '#' || op == 'F' || op == 'T' || op == '@' || op == '[')
	{
		switch (op)
		{
		case '-': unaryOps.push(Program::Op::NEG); break;
		case '+': break; // no op
		case '$': unaryOps.push(Program::Op::SIN); break;
		case '#': unaryOps.push(Program::Op::SQR); break;
		case 'F': unaryOps.push(Program::Op::FREQ); break;
		case 'T': unaryOps.push(Program::Op::TRI); break;
		case '@': unaryOps.push(Program::Op::PEK); break;
		case '[': unaryOps.push(Program::Op::GET); break;
		}
		state.parsePos++;
		op = *state;
	}

	// Check if there is parenthesis
	if (*state == '(')
	{
		state.parsePos++;
		state.parenCount++;
		if (Parse(state)) return 1;
		if (*state != ')')
		{
			// Unmatched opening parenthesis
			state.error = Program::CE_MISSING_PAREN;
			return 1;
		}
		state.parsePos++;
		state.parenCount--;

		while (!unaryOps.empty())
		{
			state.Push(unaryOps.top());
			unaryOps.pop();
		}

		return 0;
	}

	if (isalpha(*state))
	{
		const Program::Char var = *state;
		// push the address of the variable, which peek will need
		state.Push(Program::Op::NUM, var + 128);
		state.Push(Program::Op::PEK);
		state.parsePos++;
	}
	else // parse a numeric value
	{
		const Program::Char* startPtr = state.source + state.parsePos;
		Program::Char* endPtr = nullptr;
		Program::Value res = (Program::Value)strtoull(startPtr, &endPtr, 10);
		// failed to parse a number
		if (endPtr == startPtr)
		{
			state.error = Program::CE_UNEXPECTED_CHAR;
			return 1;
		}
		state.Push(Program::Op::NUM, res);
		// advance our index based on where the end pointer wound up
		state.parsePos += (endPtr - startPtr) / sizeof(Program::Char);
	}

	while (!unaryOps.empty())
	{
		state.Push(unaryOps.top());
		unaryOps.pop();
	}

	return 0;
}

static int ParseFactors(CompilationState& state)
{
	if (ParseAtom(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();

		// Save the operation and position
		Program::Char op = *state;
		if (op != '/' && op != '*' && op != '%')
		{
			return 0;
		}
		state.parsePos++;
		if (ParseAtom(state)) return 1;
		// Perform the saved operation
		if (op == '/')
		{
			state.Push(Program::Op::DIV);
		}
		else if (op == '%')
		{
			state.Push(Program::Op::MOD);
		}
		else
		{
			state.Push(Program::Op::MUL);
		}
	}
}

static int ParseSummands(CompilationState& state)
{
	if (ParseFactors(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '-' && op != '+')
		{
			return 0;
		}
		state.parsePos++;
		if (ParseFactors(state)) return 1;
		switch (op)
		{
		case '-': state.Push(Program::Op::SUB); break;
		case '+': state.Push(Program::Op::ADD); break;
		}
	}
}

static int ParseCmpOrShift(CompilationState& state)
{
	if (ParseSummands(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '<' && op != '>')
		{
			return 0;
		}
		state.parsePos++;
		Program::Char op2 = *state;
        // not a bitshift, so do compare
		if (op2 != op)
		{
            if (ParseSummands(state)) return 1;
			switch(op)
            {
                case '<': state.Push(Program::Op::CLT); break;
                case '>': state.Push(Program::Op::CGT); break;
            }
		}
        else
        {
            // is a bitshift, so eat it and continue
            state.parsePos++;
            if (ParseSummands(state)) return 1;
            switch (op)
            {
            case '<': state.Push(Program::Op::BSL); break;
            case '>': state.Push(Program::Op::BSR); break;
            }
        }
	}
}

static int ParseAND(CompilationState& state)
{
	if (ParseCmpOrShift(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '&')
		{
			return 0;
		}
		state.parsePos++;
		if (ParseCmpOrShift(state)) return 1;
		state.Push(Program::Op::AND);
	}
}

static int ParseXOR(CompilationState& state)
{
	if (ParseAND(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '^')
		{
			return 0;
		}
		state.parsePos++;
		if (ParseAND(state)) return 1;
		state.Push(Program::Op::XOR);
	}
}

static int ParseOR(CompilationState& state)
{
	if (ParseXOR(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '|')
		{
			return 0;
		}
		state.parsePos++;
		if (ParseXOR(state)) return 1;
		state.Push(Program::Op::OR);
	}
}

static int ParseTRN(CompilationState& state)
{
    if(ParseOR(state)) return 1;
    for(;;)
    {
		state.SkipWhitespace();
        Program::Char op = *state;
        if ( op != '?' )
        {
            return 0;
        }
        state.parsePos++;
        if(Parse(state)) return 1;
		state.SkipWhitespace();
        op = *state;
        if ( op != ':' )
        {
            state.error = Program::CE_UNEXPECTED_CHAR;
            return 1;
        }
        state.parsePos++;
        if (Parse(state)) return 1;
		// if the statement terminated in a semi-colon we will have a POP on the end of the list.
		// we need that stack value to be able to evaluate the ternary statement,
		// but we need to retain the POP instruction to account for the semi-colon.
		if (state.ops.back().code == Program::Op::POP)
		{
			state.ops.pop_back();
			state.Push(Program::Op::TRN);
			state.Push(Program::Op::POP);
		}
		else
		{
			state.Push(Program::Op::TRN);
		}
    }
}

static int ParsePOK(CompilationState& state)
{
	if (ParseTRN(state)) return 1;
	for (;;)
	{
		state.SkipWhitespace();
		Program::Char op = *state;
		if (op != '=')
		{
			return 0;
		}
		state.parsePos++;
		// PEK and GET work by popping a value from the stack to use as the lookup address.
		// so when we want to POK or PUT, we can use that same address to know where in memory to assign the result of the right side.
		// we just need to remove the existing instruction so the address will still be on the stack after the instructions generated by ParseTRN complete.
		// we require the presence of a PEK or GET instruction because we don't want to allow statements like '5 = 4'
		// instead, we accept '@5 = 4', which means "set memory address 5 to the value 4".
		// similarly, 'a = 4' will assign 4 to the memory address reserved for the variable 'a' (see ParseAtom).
		// [0] = 5 will put the value 5 into first output result
		Program::Op::Code code = state.ops.back().code;
		if (code == Program::Op::PEK || code == Program::Op::GET)
		{
			state.ops.pop_back();
		}
		else
		{
			state.error = Program::CE_ILLEGAL_ASSIGNMENT;
			return 1;
		}
		if (ParseTRN(state)) return 1;
		switch (code)
		{
		case Program::Op::PEK: 
			state.Push(Program::Op::POK); 
			break;

		case Program::Op::GET: 
			state.Push(Program::Op::PUT); 
			// and make sure there's a closing ] to eat
			//state.SkipWhitespace();
			//if (*state == ']')
			//{
			//	state.parsePos++;
			//}
			//else
			//{
			//	state.error = Program::CE_MISSING_BRACKET;
			//	return 1;
			//}
			break;
		}
	}
}

// we start here so that we don't have to change the forward declare for ParseAtom when we add another level
static int Parse(CompilationState& state)
{
	state.parseDepth++;
    while(*state != '\0')
    {
        if (ParsePOK(state)) return 1;
        // check for statement termination
		state.SkipWhitespace();
        if (*state != ';')
        {
			state.parseDepth--;
            return 0;
        }
		// if we have recursed into Parse due to opening parens
		// or due to parsing a section of a ternary operator,
		// we should throw an error if we encounter a semi-colon
		// because those constructs will not evaulate correctly
		// if a POP appears in the middle of the instructions.
		if (state.parseDepth != 1)
		{
			state.error = Program::CE_ILLEGAL_STATEMENT_TERMINATION;
			return 1;
		}
        state.parsePos++;
        state.Push(Program::Op::POP);
		// skip space immediately after statement termination
		// in case this is the last symbol of the program but there is trailing whitespace
		state.SkipWhitespace();
    }
    
    return 0;
}

Program* Program::Compile(const Char* source, CompileError& outError, int& outErrorPosition)
{
	Program* program = nullptr;
	CompilationState state(source);

	Parse(state);

	// final error checking
	if (state.error == CE_NONE)
	{
		// Now, expr should point to '\0', and _paren_count should be zero
		if (state.parenCount != 0 || *state == ')')
		{
			state.error = Program::CE_MISSING_PAREN;
		}
		else if (*state != '\0')
		{
			state.error = Program::CE_UNEXPECTED_CHAR;
		}
	}

	// now create a program or don't
	if (state.error == CE_NONE)
	{
		outError = Program::CE_NONE;
		outErrorPosition = -1;
		program = new Program(state.ops);
	}
	else
	{
		outError = state.error;
		outErrorPosition = state.parsePos;
	}

	return program;
}

Program::Program(const std::vector<Op>& inOps)
: ops(inOps)
{
	// default sample rate so the F operator will function
	Set('~', 44100);
}

Program::~Program()
{
}
#pragma endregion

//////////////////////////////////////////////////////////////////////////
// EXECUTION
//////////////////////////////////////////////////////////////////////////
#pragma region Execution

const char * Program::GetErrorString(Program::RuntimeError error)
{
	switch (error)
	{
	case RE_NONE:
		return "None";
	case RE_DIVIDE_BY_ZERO:
		return "Divide by zero";
	case RE_MISSING_OPERAND:
		return "Missing operand";
	case RE_MISSING_OPCODE:
		return "Unimplemented opcode";
	case RE_INCONSISTENT_STACK:
		return "Inconsistent stack";
	case RE_EMPTY_PROGRAM:
		return "Empty program (instruction count is zero)";
	case RE_GET_OUT_OF_BOUNDS:
		return "Input access is out of bounds";
	case RE_PUT_OUT_OF_BOUNDS:
		return "Output access is out of bounds";
	default:
		return "Unknown";
	}
}

Program::RuntimeError Program::Run(Value* results, const size_t size)
{
	RuntimeError error = RE_NONE;
	const uint64_t icount = GetInstructionCount();
	if (icount > 0)
	{
		Value stackResult = 0;
		bool  didPut = false;

		for (uint64_t i = 0; i < icount && error == RE_NONE; ++i)
		{
            if ( ops[i].code == Op::POP )
            {
                if ( stack.size() > 0 )
                {
					stackResult = stack.top();
                    stack.pop();
                }
                else
                {
                    error = RE_INCONSISTENT_STACK;
                }
            }
            else
            {
				didPut = didPut || ops[i].code == Op::PUT;
                error = Exec(ops[i], results, size);
            }
		}

        // under error-free execution we should have either 1 or 0 values in the stack.
        // 1 when a program terminates with the result of an expression (eg: t*Fn)
        // 0 when a program terminates with a POP (eg: t*Fn;)
        // in the case of the POP, the value of the expression will already be in result.
		if (error == RE_NONE)
		{
            if ( stack.size() == 1 )
            {
				stackResult = stack.top();
                stack.pop();
            }
            else if ( stack.size() > 1 )
            {
                error = RE_INCONSISTENT_STACK;
            }

			// implicitly output to all channels if they did not explicitly put somewhere
			if (!didPut)
			{
				for (int i = 0; i < size; ++i)
				{
					results[i] = stackResult;
				}
			}
		}
		
        // clear the stack so it doesn't explode in size due to continual runtime errors
        while( stack.size() > 1 )
		{
            stack.pop();
		}
	}
	else
	{
		error = RE_EMPTY_PROGRAM;
	}

	return error;
}

#define POP1 if ( stack.size() < 1 ) goto bad_stack; Value a = stack.top(); stack.pop();
#define POP2 if ( stack.size() < 2 ) goto bad_stack; Value b = stack.top(); stack.pop(); Value a = stack.top(); stack.pop();
#define POP3 if ( stack.size() < 3 ) goto bad_stack; Value c = stack.top(); stack.pop(); Value b = stack.top(); stack.pop(); Value a = stack.top(); stack.pop();

// perform the operation
Program::RuntimeError Program::Exec(const Op& op, Value* results, size_t size)
{
	RuntimeError error = RE_NONE;
	switch (op.code)
	{
		// no operands - result is pushed to the stack
		case Op::NUM: 
			stack.push(op.val); 
			break;		

		// one operand - operand value is popped from the stack, result is pushed back on 
		case Op::PEK:
		{
			POP1;
			stack.push(Peek(a));
		}
		break;

		case Op::GET:
		{
			POP1;
			Value v = 0;
			if (a < size)
			{
				v = results[a];
			}
			else
			{
				error = RE_GET_OUT_OF_BOUNDS;
			}
			stack.push(v);
		}
		break;

		case Op::NEG:
		{
			POP1;
			stack.push(-a);
		}
		break;

		case Op::SIN:
		{
			POP1;
			Value r = Get('r');
			Value hr = r / 2;
			r += 1;
			double s = sin(2 * M_PI * ((double)(a%r) / r));
			stack.push(Value(s*hr + hr));
		}
		break;

		case Op::SQR:
		{
			POP1;
			const Value r = Get('r');
			const Value v = a%r < r / 2 ? 0 : r - 1;
			stack.push(v);
		}
		break;

		case Op::FREQ:
		{
			POP1;
			if (a == 0)
			{
				stack.push(0);
			}
			else
			{
				double f = round(3 * pow(2.0, (double)a / 12.0) * (44100.0 / Get('~')));
				stack.push((Value)f);
			}
		}
		break;

		case Op::TRI:
		{
			POP1;
			a *= 2;
			const Value r = Get('r');			
			const Value v = a*((a / r) % 2) + (r - a - 1)*(1 - (a / r) % 2);
			stack.push(v);
		}
		break;

		// two operands - both are popped from the stack, result is pushed back on
		case Op::POK:
		{
			POP2;
			Poke(a, b);
			stack.push(b);
		}
		break;

		case Op::PUT:
		{
			POP2;
			if (a == -1)
			{
				for (int i = 0; i < size; ++i)
				{
					results[i] = b;
				}
			}
			else if (a < size)
			{
				results[a] = b;
			}
			else
			{
				error = RE_PUT_OUT_OF_BOUNDS;
			}
			stack.push(b);
		}
		break;

		case Op::MUL: 
		{
			POP2;
			stack.push(a*b);
		}
		break;

		case Op::DIV:
		{
			POP2;
			Value v = 0;
			if (b) { v = a / b; }
			else { error = RE_DIVIDE_BY_ZERO; }
			stack.push(v);
		}
		break;

		case Op::MOD: 
		{
			POP2;
			Value v = 0;
			if (b) { v = a%b; }
			else { error = RE_DIVIDE_BY_ZERO; }
			stack.push(v);
		}
		break;

		case Op::ADD: 
		{
			POP2;
			stack.push(a + b);
		}
		break;

		case Op::SUB: 
		{
			POP2;
			stack.push(a - b);
		}
		break;

		case Op::BSL: 
		{
			POP2;
			stack.push(a << b);
		}
		break;

		case Op::BSR: 
		{
			POP2;
			stack.push(a >> (b % 64));
		}
		break;

		case Op::AND: 
		{
			POP2;
			stack.push(a&b);
		}
		break;

		case Op::OR: 
		{
			POP2;
			stack.push(a | b);
		}
		break;

		case Op::XOR: 
		{
			POP2;
			stack.push(a^b);
		}
		break;
            
        case Op::CLT:
        {
            POP2;
            stack.push(a<b);
        }
        break;
            
        case Op::CGT:
        {
            POP2;
            stack.push(a>b);
        }
        break;
            
        case Op::TRN:
        {
            POP3;
            stack.push(a ? b : c);
        }
        break;

		// perform a no-op, but set the error as a result
		default:
		{
			error = RE_MISSING_OPCODE;
		}
		break;

		bad_stack:
		{
			error = RE_MISSING_OPERAND;
		}
		break;
	}

	return error;
}

Program::Value Program::Get(const Char var) const
{
	return Peek((Value)var + 128);
}

void Program::Set(const Char var, const Value value)
{
	Poke((Value)var + 128, value);
}


Program::Value Program::Peek(const Value address) const
{
	// peeks wrap around so we never go outside of our memory space
	return mem[address%kMemorySize];
}


void Program::Poke(const Value address, const Value value)
{
	// pokes wrap around so we never go outside of our memory space
	mem[address%kMemorySize] = value;
}

#pragma endregion
