#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "SymbolTable.h"
#include "ASTWalker.h"
#include "jmm_parse.h"
#include "CodeGenerator.h"

const char* CodeGenerator::r[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

CodeGenerator::CodeGenerator(std::string output_file)
{
  this->m_out = output_file;
  this->glob_offs = 0;
  this->label_count = 1;
  this->string_label_count = 0;
  
  this->m_inst = new std::vector<std::string>();
  this->m_globals = new std::vector<std::string>();
}


void CodeGenerator::add_instruction(std::string instruction)
{
  add_instruction(instruction, m_inst);
}

void CodeGenerator::add_instruction(std::string instruction, int tabs)
{
  std::string newStr = "";
  for (int i = 0; i < tabs; i++)
    newStr.append("\t");
  newStr.append(instruction);
  add_instruction(newStr);
}

void CodeGenerator::add_instruction(std::string instruction, std::vector< std::string >* instrs, int tabs)
{
  if (instruction == "") return;
  
  std::string newStr = "";
  for (int i = 0; i < tabs; i++)
    newStr.append("\t");
  newStr.append(instruction);
  add_instruction(newStr, instrs);
}

void CodeGenerator::add_instruction(std::string instruction, std::vector< std::string >* instrs)
{
  if (instruction == "") return;
  
  instruction.append("\n");
  instrs->push_back(instruction);  
}

void CodeGenerator::add_instructions(CodeBlock* dst, CodeBlock* src)
{ 
  // Insert all instructions.
  dst->insert(dst->end(), src->begin(), src->end());
}

void CodeGenerator::add_comment(std::string str)
{
  std::string comment = "@ ";
  add_instruction(comment.append(str));
}

std::string CodeGenerator::label(std::string lbl)
{
  return lbl.append(":");
}

void CodeGenerator::add_label(std::string str, Symbol* sym)
{
  labels[sym] = str;
  sym->label = str;
}

std::string CodeGenerator::next_label()
{
  std::stringstream ss;
  ss << "L" << this->label_count++;
  return ss.str();
}

// Creates the intro code to the program. Required to get it all started.
void CodeGenerator::create_intro()
{
  // Set up the text section.
  add_instruction(".text", 1);
  
  // Entry point.
  add_instruction(".globl _start", 1);
  add_instruction(".align 4", 1);
  add_instruction("_start:");
  add_comment("Set up initial stack pointer.");
  
  // Load the stack pointer.
  add_instruction(instr_2("adrl", "sp", "prog_end"), 1);
  
  // Set r0 to 1.
  add_instruction(instr_2("mov", r[0], num(1)), 1);
  
  // Shift left by STACK_POWER
  add_instruction(instr_2("lsl", r[0], num(STACK_POWER)), 1);
  
  // Add it to the stack pointer.
  add_instruction(instr_2("add", "sp", r[0]), 1);
  
  // Store globals section for later use.
  add_instruction(instr_2("adrl", "ip", GLOBAL_SECTION), 1);
  
  // Branch to the entry point code.
  add_instruction(instr_1("bl", "L0"), 1);
}

// Writes any end-code that is required for all programs.
void CodeGenerator::create_end()
{
  std::ifstream in("rts.s");
  
  if (in) {    
    std::stringstream ss;
    ss << in.rdbuf() << "\n";
    in.close();
    
    m_inst->push_back(ss.str());
  }
  
  add_instruction(".align 4", 1);
  add_instruction(label(GLOBAL_SECTION));
  add_global("prog_end = .");
}

int CodeGenerator::add_global(int val, std::string comment)
{
  std::stringstream ss;
  ss << ".word " << val << "\t\t@ " << comment << "\n";
  glob_offs += WORD_SIZE;
  m_globals->push_back(ss.str());
  return (glob_offs - WORD_SIZE);
}

void CodeGenerator::add_global(std::string str)
{
  m_globals->push_back(str.append("\n"));
}

std::string CodeGenerator::instr_1(std::string instr, std::string a1)
{
  std::stringstream ss;
  ss << instr << "\t" << a1;
  return ss.str();
}

// A 2 parameter instruction.
std::string CodeGenerator::instr_2(std::string instr, std::string a1, std::string a2)
{
  std::stringstream ss;
  ss << instr << "\t" << a1 << ", " << a2;
  return ss.str();
}

// 3 parameter instruction.
std::string CodeGenerator::instr_3(std::string instr, std::string a1, std::string a2, std::string a3)
{
  std::stringstream ss;
  ss << instr << "\t" << a1 << ", " << a2 << ", " << a3;
  return ss.str();
}

// Converts an integer to a number value.
std::string CodeGenerator::num(int n)
{
  std::stringstream ss;
  ss << "#" << n;
  return ss.str();
}

std::string CodeGenerator::num(std::string n)
{
  std::stringstream ss;
  ss << "#" << n;
  return ss.str();
}

std::string CodeGenerator::get_next_reg(RegisterSet* regs, bool arg_reg)
{
  std::string reg = "";
  
  bool found = false;
  
  int low_reg = arg_reg ? 0 : SCRATCH_REG_START;
  int high_reg = arg_reg ? 4 : SCRATCH_REG_END;
  
  for (int i = low_reg; i < high_reg; i++) {
    for (unsigned int j = 0; j < regs->size(); j++) {
      if (r[i] == regs->at(j)) {
        found = true;
      }
    }
    
    // If this reg wasn't found in the list, we can use it.
    if (found == false) {
      reg = r[i];
      break;
    }
    
    found = false;
  }
  
  if (reg != "") {
    regs->push_back(reg);
  }
  
  return reg;
}

bool CodeGenerator::move_to_arg_regs(CodeBlock* code, RegisterSet* regs, int size)
{
  if (size > 4) {
    std::cerr << "ERROR: Cannot handle more than 4 arguments per function." << std::endl;
    return false;
  }
  
  if ((int)regs->size() < size) {
    std::cerr << "ERROR: Too few registers to handle!" << std::endl;
  }
  
  if (size == 0)
    return true;
  
  int i = size - 1;
  while (i >= 0) {
    std::string reg = regs->back();
    regs->pop_back();
    std::string ar = r[i];
    
    add_instruction(move(ar, reg), code, 1); 
    i--;
  }

  return true;
}

bool CodeGenerator::is_rts(std::string rts)
{
  return (rts == "prints" || rts == "getchar" || rts == "printi" || rts == "printc" ||
    rts == "printb" || rts == "halt");
}

std::string CodeGenerator::load_identifier(Symbol* sym, RegisterSet* regs)
{
  std::string load_instr;
  //if (sym->arg_sym)
  //  load_instr = move(get_next_reg(regs, false), r[sym->arg_reg]);
  //else
    load_instr = instr_2("ldr", get_next_reg(regs, false), id_load_string(sym));

  return load_instr;
}

std::string CodeGenerator::id_load_string(Symbol* sym)
{
  std::string mem;
  if (sym->global) {
    // Handle global variable
    mem = "ip";
  } else {
    // Offset based on stack.
    mem = "sp";
  }
  
  return mem_offs(mem, sym->offset);
}

std::string CodeGenerator::mem_offs(std::string ref, int amt)
{
  std::stringstream ss;
  ss << "[" << ref << "," << num(amt) << "]";
  return ss.str();
}

std::string CodeGenerator::get_math_op_instr(int op)
{
  switch (op) {
    case MUL: return "mul";
    case DIV: return "sdiv";
    case ADD: return "add";
    case SUB: return "sub";
  }
  
  return "";
}

std::string CodeGenerator::get_cond_op_instr(int op)
{
  switch(op) {
    case AND:
    case OR:
    case EQ: return "moveq";
    case GE: return "movge";
    case LE: return "movle";
    case NE: return "movne";
    case LESS: return "movlt";
    case GREAT: return "movgt";
  }
  
  return "";
}

std::string CodeGenerator::move(std::string reg, std::string val)
{
  return instr_2("mov", reg, val);
}

// Turns a list of registers into a instruction to save them.
std::string CodeGenerator::do_regs(bool store, std::vector< std::string > regs)
{
  std::stringstream ss;
  ss << "{";
  /*
  for (unsigned int i = 0; i < regs.size(); i++) {
    ss << regs[i] << ",";
  }
  */
  // Store all registers.
  ss << "r4-r11,";
  if (store) ss << "lr";
  else ss << "pc";
  ss << "}";
  
  if (store) {
    return instr_2("stmfd", "sp!", ss.str());  
  } else {
    return instr_2("ldmfd", "sp!", ss.str());  
  }
}

void CodeGenerator::add_strings(ASTNode* head, CodeBlock* decl)
{
  std::stringstream ss;
  
  // Recurse down the tree and find all string nodes.
  if (head->getType() == LITERAL_NODE) {
    LiteralNode* lit = (LiteralNode*)head;
    if (lit->getLiteralType() == STRING) {
      ss << "LS" << string_label_count++;
      
      string_table[lit] = ss.str();
      add_instruction(instr_1(".align", "4"), decl, 1);
      add_instruction(label(ss.str()), decl);
      
      std::stringstream ss_2;
      ss_2 << "\"" << lit->getToken()->val << "\""; 
      add_instruction(instr_1(".asciz", ss_2.str()), decl, 1);
      add_instruction(".word 0", decl, 1);
    }
  }
  
  NodeChildren children = head->getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    add_strings(children[i], decl);
  }
}

void CodeGenerator::prealloc_func_labels(ASTNode* head)
{
  std::string type = head->getType();
  
  if (type == FUNCTION_HEAD_NODE) {
    FunctionHeaderNode* header = (FunctionHeaderNode*)head;
    std::string lbl = next_label();
    add_label(lbl, header->getDeclaration()->getID()->getSymbol());    
    return;
  } else if (type == MAIN_FUNC_NODE) {
    MainFunctionNode* mfn = (MainFunctionNode*)head;
    std::string lbl = "L0";
    add_label(lbl, mfn->getDeclaration()->getID()->getSymbol());
    return;
  }
  
  NodeChildren children = head->getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    prealloc_func_labels(children[i]);
  }
}

bool CodeGenerator::_generate(ASTNode* node, CodeBlock* code, RegisterSet* regs)
{
  std::string type = node->getType();
  
  // When we find a block node we create a new vector to push
  // instructions on to. This allows us to create block-specific 
  // code segments.
  if (type == BLOCK_NODE) {
    std::vector<std::string> block_code;
    
    std::vector<ASTNode*> children = node->getChildren();
    for (unsigned int i = 0; i < children.size(); i++) {
      _generate(children.at(i), &block_code, regs);  
    }
    
    // Append the block's code to the main code.
    code->insert(code->end(), block_code.begin(), block_code.end());
  }
  
  // Handle a function definition.
  if (type == FUNCTION_HEAD_NODE) {
    // Get the block for the function.
    FunctionHeaderNode* fhn = (FunctionHeaderNode*)node;
    ASTNode* block = fhn->getCode();
    
    // Create a label for this function.
    std::string lbl = fhn->getDeclaration()->getID()->getSymbol()->label;
    
    // Find all locals.
    std::string local_alloc = allocate_locals(false, fhn);
    std::string local_dealloc = allocate_locals(true, fhn);
    
    // Set up the return data so we can return multiple times out of a 
    // function.
    std::string ret_str = local_dealloc;
    ret_str.append("\n\t");
    ret_str.append(do_regs(false, *regs));
    fhn->getDeclaration()->getID()->getSymbol()->aux = ret_str;
        
    CodeBlock* strings = new CodeBlock;
    add_strings(block, strings);
    
    // Generate the code for this function's block.
    std::vector<std::string>* block_code = new std::vector<std::string>();
    RegisterSet* func_regs = new RegisterSet;
    _generate(block, block_code, func_regs);
    
    add_instruction(".align 4", code, 1);
    add_instruction(label(lbl), code);
    add_instruction(do_regs(true, *regs), code, 1);
    add_instruction(local_alloc, code, 1);
    save_params(fhn->getDeclaration()->getParams()->getNodes(), code);
    add_instruction(".ltorg", code, 1);
    // Copy the block instructions in.
    add_instructions(code, block_code);
    
    // If we're in a non-void function, this is handled
    // by the return statement. So check if we're void.
    //if (fhn->getFunctionType() == VOID)
      add_instruction(ret_str, code, 1);
    
    add_instructions(code, strings);
  }
  
  if (type == MAIN_FUNC_NODE) {
    // Set up the main function.
    MainFunctionNode* mfn = (MainFunctionNode*)node;
    MainFunctionDeclarationNode* mfd = mfn->getDeclaration();

    ASTNode* block = mfn->getCode();
    
    // Set up the symbol.
    std::string lbl = mfd->getID()->getSymbol()->label;
    
    // Find all locals.
    std::string local_alloc = allocate_locals(false, mfn);
    std::string local_dealloc = allocate_locals(true, mfn);   
    
    CodeBlock* strings = new CodeBlock;
    add_strings(block, strings);
    
    // Generate the code for this function's block. Give it a new
    // set of registers to work with.
    CodeBlock* block_code = new CodeBlock;
    RegisterSet* block_regs = new RegisterSet;
    _generate(block, block_code, block_regs);
    
    add_instruction(".align 4", code, 1);
    add_instruction(label(lbl), code);
    add_instruction(local_alloc, code, 1);
    // Copy the block instructions in.
    add_instructions(code, block_code);
    add_instruction(local_dealloc, code, 1);
    add_instruction(instr_1("bl", "Lhalt"), code, 1);
    add_instruction(".ltorg", code, 1);
    add_instructions(code, strings);
  }
  
  // Handle function invokation
  if (type == FUNCTION_INVOKE_NODE) {
    // For invokation we want to load in relevant registers from memory and set up the branch.
    FunctionInvokationNode* inv = (FunctionInvokationNode*)node;
    
    // Parse the arguments
    FunctionParamNode* params = inv->getParams();
    std::vector<std::string>* param_instrs = new std::vector<std::string>;
    std::vector<ASTNode*> p = params->getNodes();
    for (unsigned int i = 0; i < p.size(); i++) {
      parse_expression(p[i], param_instrs, regs);
    }
    move_to_arg_regs(param_instrs, regs, p.size());
    
    std::string lbl;
    if (is_rts(inv->getID()->getToken()->val)) {
      lbl = "L";
      lbl.append(inv->getID()->getToken()->val);
      lbl = instr_1("bl", lbl);
    } else {
      // Branch to the function.
      lbl = instr_1("bl", labels[inv->getID()->getSymbol()]);
    }
    
    if (inv->getID()->getToken()->val == "getchar")
      regs->push_back(r[0]);
    
    add_instructions(code, param_instrs);
    add_instruction(lbl, code, 1);    
    
    if (inv->getID()->getSymbol()->type != SYM_NONE) {
      add_instruction(move(get_next_reg(regs, false), r[0]), code, 1);
    }
  }
  
  // Parse assignment nodes.
  if (type == ASSIGNMENT_NODE) {
    AssignmentNode* as = (AssignmentNode*)node;

    // Allocate a new code block and parse the right hand side expression.
    CodeBlock* rhs_code = new CodeBlock();
    parse_expression(as->getRHS(), rhs_code, regs);
    
    // The register at the back of the regs list is our result.
    add_instructions(code, rhs_code);
    add_instruction(instr_2("str", regs->back(), 
                    id_load_string(as->getLHS()->getSymbol())), code, 1);
    regs->pop_back();   // Remove the result register.
  }
  
  if (type == RESERVED_NODE) {
    ReservedNode* res = (ReservedNode*)node;
    int res_type = res->getReservedType();
    NodeChildren children = res->getChildren();
    
    switch (res_type) {
      case T_IF:
      case T_IFELSE:
      {
        // If has 2 children, first is expression (boolean) and second is statement.
        // ifelse has 3 children, third child is the else's statements.
        ExpressionNode* expr = (ExpressionNode*)children[0];
        ASTNode* if_code = children[1];
        
        // Parse the boolean expression.
        parse_expression(expr, code, regs);
        
        std::string skip_lbl = next_label();
        std::string else_skip;
        if (res_type == T_IFELSE)
          else_skip = next_label();
        
        // Test the expression result.
        add_instruction(instr_2("cmp", regs->back(), num(TRUE)), code, 1);
        add_instruction(instr_1("bne", skip_lbl), code, 1);
        regs->pop_back();	// Remove result register.
        
        // Parse the statement code.
        _generate(if_code, code, regs);
        if (res_type == T_IFELSE)
          add_instruction(instr_1("b", else_skip), code, 1);
        
        add_instruction(label(skip_lbl), code);
        
        if (res_type == T_IFELSE) {
          _generate(children[2], code, regs);
          add_instruction(label(else_skip), code);
        }
        
        break;
      }
      case T_WHILE:
      {
        // The boolean expression.
        ExpressionNode* expr = (ExpressionNode*)children[0];
        ASTNode* block = children[1]; // Code Block.
        
        // Label to skip past while loop.
        std::string begin_lbl = next_label();
        std::string end_lbl = next_label();
        res->aux = end_lbl;

        // Test the expression result.
        add_instruction(label(begin_lbl), code);       
        
        // Parse the boolean expression.
        parse_expression(expr, code, regs);
        
        add_instruction(instr_2("cmp", regs->back(), num(TRUE)), code, 1);
        add_instruction(instr_1("bne", end_lbl), code, 1);
        regs->pop_back();	// Remove result register.
        
        _generate(block, code, regs);
        add_instruction(instr_1("b", begin_lbl), code, 1);
        add_instruction(label(end_lbl), code);
        
        break;
      }
      case T_BREAK:
      {
        // Recurse up until we find the while loop.
        ASTNode* parent = res->getParent();
        while (parent != NULL) {
          if (parent->getType() == RESERVED_NODE)
            if (((ReservedNode*)parent)->getReservedType() == T_WHILE)
              break;
          parent = parent->getParent();
        }
        
        if (parent == NULL)
          return false;
        
        // Get the end label.
        std::string end_lbl = ((ReservedNode*)parent)->aux;
        add_instruction(instr_1("b", end_lbl), code, 1);
        
        break;
      }
      case T_RETURN:
      {
        ASTNode* parents = node->getParent();

        /* Hack to get the return registers. */
        while (parents != NULL && parents->getType() != FUNCTION_HEAD_NODE) {
          parents = parents->getParent();
          if (parents == NULL) {
            add_comment("FAIL: Could not return.");
            break;
          }
        }

        // Return regs are stored in aux.
        FunctionHeaderNode* head = (FunctionHeaderNode*)parents;
        std::string ret_regs = head->getDeclaration()->getID()->getSymbol()->aux;
        if (children.size() == 1) {
          ExpressionNode* ret_val = (ExpressionNode*)children.back();

          parse_expression(ret_val, code, regs);
          add_instruction(move(r[0], regs->back()), code, 1);
          regs->pop_back();
        } 
        
        add_instruction(ret_regs, code, 1);
        
        break;
      }
    }
  }
  
  return true;
}

bool CodeGenerator::parse_expression(ASTNode* expr, CodeBlock* code, RegisterSet* regs, bool can_const ) {
  std::string type = expr->getType();
  
  if (type == LITERAL_NODE) {
    LiteralNode* lit = (LiteralNode*)expr;
    
    std::string mov;
    int v = 0;
    switch (lit->getLiteralType()) {
      case BOOLEAN:
        if (lit->getToken()->type == "true")
          v = 1;
        
        if (!can_const) {
          mov = move(get_next_reg(regs, false), num(v));
          add_instruction(mov, code, 1); 
        } else {
          regs->push_back(num(v));
        }
        break;
      case INT:
      {
        int val = atoi(lit->getToken()->val.c_str());
        if (val > 255) {
          std::stringstream ss;
          ss << "ldr\t" << get_next_reg(regs, false) << ", " << "=" << val;
          add_instruction(ss.str(), code, 1);
          break;
        }
        
        if (!can_const) {
          mov = move(get_next_reg(regs, false), num(lit->getToken()->val));
          add_instruction(mov, code, 1);
        } else {
          regs->push_back(num(lit->getToken()->val));
        }
        break;
      }
      case STRING:
        // Handle strings here.
        std::string str_lbl = string_table[lit];
        
        add_instruction(instr_2("adrl", get_next_reg(regs, false), str_lbl), code, 1);
                
        break;
    }
  }
  
  if (type == IDENTIFIER_NODE) {
    Symbol* sym = ((IdentifierNode*)expr)->getSymbol();
    add_instruction(load_identifier(sym, regs), code, 1);
  }
  
  if (type == FUNCTION_INVOKE_NODE) {
    FunctionInvokationNode* inv = (FunctionInvokationNode*)expr;
    _generate(expr, code, regs);
    
    // only set up the return value if there is something to be returned
    // return value is stored in r0.
    if (inv->getID()->getSymbol()->type != SYM_NONE) {
      //regs->push_back(r[0]);
    }
  }
  
  if (type == OPERATION_NODE) {
    OperationNode* op = (OperationNode*)expr;
    int op_type = op->getOperation();      
    
    // Handle left side, then right side. consider using alg
    // from piazza if enough time at end.
    parse_expression(op->getLHS(), code, regs);
    std::string reg_left = regs->back();
    parse_expression(op->getRHS(), code, regs, true);
    std::string reg_right  = regs->back();    
    
    // Check conditional
    if (op_type >= EQ && op_type <= GREAT) {
      // Handle conditional. (Watch out for short circuiting!) */

      if (op_type == AND) {
        add_instruction(instr_2("and", reg_left, reg_right), code, 1);
        reg_right = "#1";	
      } else if (op_type == OR) {
	add_instruction(instr_2("orr", reg_left, reg_right), code, 1);
        reg_right = "#1";
      }

      std::string result_reg = get_next_reg(regs, false);
      // Clear the result register.
      add_instruction(move(result_reg, num(0)), code, 1);
      add_instruction(instr_2("cmp", reg_left, reg_right), code, 1);
      add_instruction(instr_2(get_cond_op_instr(op_type), result_reg, num(TRUE)), code, 1);
      regs->pop_back();
      regs->pop_back();
      regs->pop_back();
      regs->push_back(result_reg);
    } else if (op_type >= MUL && op_type <= SUB) { // Check mathematical
      // Handle mathematic resolution.
      std::string op_name = get_math_op_instr(op_type);    
      // Pop both of the registers.
      regs->pop_back();
      regs->pop_back();  
      
      // Handle modulus case, as it needs to be expanded into a loop.
      if (op_type == MOD) {
        // TODO: Verify that this actually works.
        // Add a label for the loop.
        std::string loop_label = next_label();
        std::string loop_end = next_label();
        add_instruction(label(loop_label), code); // Beginning of loop.
        add_instruction(instr_2("cmp", reg_left, reg_right), code, 1);
        add_instruction(instr_1("blt", loop_end), code, 1);
        add_instruction(instr_3("sub", reg_left, reg_left, reg_right), code, 1);
        add_instruction(instr_1("b", loop_label), code, 1);
        add_instruction(label(loop_end), code); // End of loop.
        regs->push_back(reg_left);
      } else {
        add_instruction(instr_3(op_name, get_next_reg(regs, false), reg_left, reg_right), code, 1); 
      }
    }
  }
  
  if (type == UNARY_OP_NODE) {
    UnaryOperationNode* unop = (UnaryOperationNode*)expr;
    int op = unop->getOperation();
    
    if (op == NOT) {
      parse_expression(unop->getRHS(), code, regs);
      
      // Check to see if the value is true, if it is reverse it.
      add_instruction(instr_2("cmp", regs->back(), num(true)), code, 1);
      add_instruction(instr_2("moveq", regs->back(), num(false)), code, 1);
      add_instruction(instr_2("movne", regs->back(), num(true)), code, 1);
    } else if (op == SUB) {
      parse_expression(unop->getRHS(), code, regs);
      
      // Negate the resultant register.
      add_instruction(instr_2("neg", regs->back(), regs->back()), code, 1);
    }
  }
  
  return true;
}

void CodeGenerator::save_params(std::vector<ASTNode*>& syms, CodeBlock* code)
{
  for (unsigned int i = 0; i < syms.size(); i++) {
    VariableDeclarationNode* vdn = (VariableDeclarationNode*)syms[i];
    IdentifierNode* id = vdn->getID();
    
    add_instruction(instr_2("str", r[i], id_load_string(id->getSymbol())), code, 1);
  }
}

void CodeGenerator::allocate_globals(ASTNode* head)
{
  if (head->getParent() != NULL) {
    return;
  }
  
  // Loop over the children of the head and find the variable declarations.
  std::vector<ASTNode*> children = head->getChildren();
  ASTNode* cur;
  for (int i = 0; i < (int)children.size(); i++) {
    cur = children.at(i);
    
    // Only allocate globals for variable declaration nodes.
    if (cur->getType() == VAR_DECL_NODE) {
      VariableDeclarationNode* decl = (VariableDeclarationNode*)cur;
      
      int o = add_global(0, decl->getID()->getToken()->toString());
      decl->getID()->getSymbol()->offset = o;
      decl->getID()->getSymbol()->global = true;
    }
  }
}

// Look through the code block to find the local variables, and create a string to
// allocate space for them on the stack.
std::string CodeGenerator::allocate_locals(bool dealloc, ASTNode* head) {
  int offs = _allocate_locals(head, 0);
  
  if (dealloc)
    return instr_2("add", "sp", num(offs));
  else
    return instr_2("sub", "sp", num(offs));
}

// Recursively searches for locals to allocate within the children.
int CodeGenerator::_allocate_locals(ASTNode* node, int offs) {
  // If it's a variable declaration, declare and return.
  if (node->getType() == VAR_DECL_NODE) {
    // Set the offset and increment the counter.
    VariableDeclarationNode *decl = (VariableDeclarationNode*)node;
    
    // Set the offset of the variable identifier.
    decl->getID()->getSymbol()->offset = offs;
    return offs + WORD_SIZE;
  }
  
  // Allocate stack space for each param.
  if (node->getType() == FUNCTION_HEAD_NODE) {
    
    FunctionHeaderNode* fhn = (FunctionHeaderNode*)node;
    NodeChildren vars = fhn->getDeclaration()->getParams()->getNodes();
    
    int o = 0;
    for (unsigned int i = 0; i < vars.size(); i++) {
      o += _allocate_locals(vars[i], o);
    }
    return offs + o;
  }

  std::vector<ASTNode*> children = node->getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    offs = _allocate_locals(children[i], offs);
  }
  
  return offs;
}

bool CodeGenerator::generate(ASTNode* head)
{
  if (!head) return false;
  
  create_intro();
  allocate_globals(head);
  prealloc_func_labels(head);
  std::vector<std::string> regs;
  bool ret = _generate(head, m_inst, &regs);
  create_end();
  
  
  
  return ret;
}

std::string CodeGenerator::getCode()
{
  std::string code = "";
  
  for (int i = 0; i < (int)m_inst->size(); i++) {
    code.append(m_inst->at(i));
  }
  
  for (int i = 0; i < (int)m_globals->size(); i++){
    code.append(m_globals->at(i));
  }
  
  return code;
}

