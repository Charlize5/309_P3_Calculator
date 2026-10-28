// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
 #include <vector> //allowed for the stacks
 
namespace { //
 
using Kind = CalcToken::Kind; // shorten the name
 
bool is_operator(Kind k) { 
    return k == Kind::Plus || k == Kind::Minus || k == Kind::Times || k == Kind::Divide; // true for the operators, false for parentheses and numbers
} 
 
int precedence(Kind k) {
    return (k == Kind::Times || k == Kind::Divide) ? 2 : 1; //PEMDAS lol 
}
 

// Returns false and sets *error on erroneous input or if the user tries to divide by zero
bool apply_top(std::vector<double>& values, std::vector<Kind>& ops, std::string* error) {
    if (ops.empty() || !is_operator(ops.back()) || values.size() < 2) {

        *error = "malformed expression"; //message out
        return false;
    }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
    Kind op = ops.back();
    ops.pop_back();
    double right = values.back(); values.pop_back();   //POP RIGHT THEN LEFT
    double left  = values.back(); values.pop_back();
 double result = 0.0;


    switch (op) {
        case Kind::Plus:   result = left + right; break;
        case Kind::Minus:  result = left - right; break;
        case Kind::Times:  result = left * right; break;
        case Kind::Divide:
            if (right == 0.0) { *error = "division by zero"; return false; }
            result = left / right;
            break;
        default:           *error = "bad expression"; return false;
    }
    values.push_back(result); // push the result back onto the values stack
    return true;
}
 
}  // namespace
 
bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out, std::string* error) {
    // check for empty expression
    if (tokens.empty()) { *error = "empty expression"; return false; }
 
    std::vector<double> values; 
    std::vector<Kind>   ops;
 
    for (const CalcToken& t : tokens) {
        switch (t.kind) {
            case Kind::Number:
                values.push_back(t.value);
                break;
 
            case Kind::LParen:
                ops.push_back(Kind::LParen);
                break;
 
            case Kind::RParen:
            // pop and apply operators until we find the matching parenthesis
            //we have to find it before another operator is applied
                while (!ops.empty() && ops.back() != Kind::LParen)
                    if (!apply_top(values, ops, error)) return false;
                if (ops.empty()) { *error = "unbalanced parentheses"; return false; }
                ops.pop_back();   // throw the parenthesis in the garbage 
                break;
 
            case Kind::Plus: 
            case Kind::Minus:
            case Kind::Times:
            case Kind::Divide:
                // ">=" (not ">") is what makes 10-4-3 evaluate left to right.
                while (!ops.empty() && is_operator(ops.back()) &&
                       precedence(ops.back()) >= precedence(t.kind))
                    if (!apply_top(values, ops, error)) return false;
                ops.push_back(t.kind);
                break;
        }
    }
 
    while (!ops.empty()) {
        if (ops.back() == Kind::LParen) { *error = "unbalanced parentheses"; return false; }
        if (!apply_top(values, ops, error)) return false; //check by message
    }
 
    if (values.size() != 1) { *error = "bad expression"; return false; }
    *out = values.back();
    return true;
}