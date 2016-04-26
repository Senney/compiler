#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <vector>
#include <string>
#include <map>

#include "ASTNode.h"
#include "ASTTypes.h"

#define STACK_POWER 20
#define WORD_SIZE 4
#define GLOBAL_SECTION "globals"
#define SCRATCH_REG_START 4
#define SCRATCH_REG_END 12

#define TRUE 1
#define FALSE 0

// Typedef our more often used containers.
typedef std::vector<std::string> RegisterSet;
typedef std::vector<std::string> CodeBlock;
typedef std::vector<ASTNode*> NodeChildren;

class CodeGenerator {
public:
  CodeGenerator(std::string output_file);
  
  bool generate(ASTNode* head);
  
  std::string getCode();
  
private:
  void add_instruction(std::string instruction);
  void add_instruction(std::string instruction, int tabs);
  void add_instruction(std::string instruction, std::vector< std::string >* instrs);
  void add_instruction(std::string instruction, std::vector< std::string >* instrs, int tabs);
  void add_instructions(CodeBlock* dst, CodeBlock* src);
  void add_comment(std::string str);
  std::string instr_1(std::string arg1, std::string arg2);
  std::string instr_2(std::string instr, std::string a1, std::string a2);
  std::string instr_3(std::string instr, std::string a1, std::string a2, std::string a3);
  std::string label(std::string lbl);
  std::string next_label();
  std::string get_next_reg(RegisterSet* regs, bool arg_reg);
  std::string load_identifier(Symbol* sym, RegisterSet* regs);
  std::string get_math_op_instr(int op);
  std::string get_cond_op_instr(int op);
  bool move_to_arg_regs(CodeBlock* code, RegisterSet* regs, int size);
  std::string mem_offs(std::string ref, int amt);
  std::string id_load_string(Symbol* sym);
  void add_label(std::string str, Symbol* sym);
  int add_global(int val, std::string comment);
  void add_global(std::string str);
  void add_strings(ASTNode* head, CodeBlock* decl);
  
  std::string move(std::string reg, std::string val);
  
  std::string do_regs(bool store, std::vector<std::string> regs);
  bool is_rts(std::string rts);
  void prealloc_func_labels(ASTNode* head);
  
  std::string num(int n);
  std::string num(std::string n);
  
  bool _generate(ASTNode* node, CodeBlock* code, RegisterSet* regs);
  bool parse_expression(ASTNode* expr, CodeBlock* code, RegisterSet* regs, bool can_const = false);
  
  // Program Generation
  void create_intro();
  void create_end();
  void allocate_globals(ASTNode* head);
  std::string allocate_locals(bool dealloc, ASTNode* head);
  int _allocate_locals(ASTNode* node, int offs);
  void save_params(std::vector< ASTNode* >& syms, CodeBlock* code);
  
  std::vector<std::string>* m_inst;
  std::vector<std::string>* m_globals;
  std::map<Symbol*, std::string> labels;
  std::map<LiteralNode*, std::string> string_table;
  int glob_offs;
  int label_count;
  int string_label_count;
  std::string m_out;
  
  // Register names.
  static const char* r[];
};

#endif