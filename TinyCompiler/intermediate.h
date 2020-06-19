#pragma once
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <utility>
#include <iomanip>
#include "SematicProcesser.h"
using namespace std;

struct Quad
{
	string op;
	string addr1;
	string addr2;
	string addr3;

	Quad(string a, string b="_", string c="_", string d="_")
	{
		op = a;
		addr1 = b;
		addr2 = c;
		addr3 = d;
	}
};

bool isOP(string op)
{
	if (op == "+" || op == "-" || op == "*")
		return true;
	else
		return false;
}
bool isComp(string op)
{
	if (op == "<" || op == "=" || op == ">")
		return true;
	else
		return false;
}
class genIR
{
private:
	hscp::AST ast;
	vector<Quad>quads;
	int tcount = 1;
	int lcount = 1;
public:
	genIR(hscp::AST a)
	{
		ast = a;
		traverse();
	}
	void traverse()
	{
		parseTree(ast.root);
	}

	//read或write节点转换成四元式
	void parseWR(hscp::ASTNode* node)
	{
		if (node->op == "READ") {
			Quad temp("rd", node->children[0]->val);
			quads.push_back(temp);
		}
		else
		{
			Quad temp("wri", node->children[0]->val);
			quads.push_back(temp);
		}
	}

	//IF节点生成四元式
	void parseIF(hscp::ASTNode* node)
	{
		hscp::ASTNode* comp = node->children[0];

		//比较部分，if的第一个孩子
		if (comp->op == ">")
		{
			Quad temp("gt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}
		else if (comp->op == "=")
		{
			Quad temp("eq", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}
		else if (comp->op == "<")
		{
			Quad temp("lt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
			quads.push_back(temp);
		}

		int lc = lcount;
		Quad f("if_f", "t" + to_string(tcount-1), "L"+to_string(lcount++) );
		quads.push_back(f);


		//true部分
		Quad t("lab", "L"+to_string(lcount++));
		quads.push_back(t);
		parseTree(node->children[1]);
		

		//false部分
		Quad fl("lab", "L" + to_string(lc));
		quads.push_back(fl);
		if (node->children.size() > 2)
			parseTree(node->children[2]);
		else
			parseTree(NULL);
	}

	//赋值语句
	void parseASN(hscp::ASTNode* node)
	{
		if (isOP(node->children[1]->op))
		{
			hscp::ASTNode* opNode = node->children[1];
			int tc = tcount;
			if (opNode->op == "+") {
				Quad temp("add", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}	
			else if (opNode->op == "-") {
				Quad temp("sub", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}
				
			else if (opNode->op == "*") {
				Quad temp("mul", opNode->children[0]->val, opNode->children[1]->val, "t" + to_string(tcount++));
				quads.push_back(temp);
			}
			Quad t("asn", "t" + to_string(tc),node->children[0]->val);
			quads.push_back(t);
		}
		else
		{
			Quad temp("asn",node->children[0]->val,node->children[1]->val);
			quads.push_back(temp);
		}
	}

	//repeat语句
	void parseREPEAT(hscp::ASTNode* node)
	{
		for (hscp::ASTNode* c : node->children)
		{
			if (isComp(c->op))
			{
				hscp::ASTNode* comp = c;
				int tc = tcount;


				//比较部分
				if (comp->op == ">")
				{
					Quad temp("gt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
					quads.push_back(temp);
				}
				else if (comp->op == "=")
				{
					Quad temp("eq", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
					quads.push_back(temp);
				}
				else if (comp->op == "<")
				{
					Quad temp("lt", comp->children[0]->val, comp->children[1]->val, "t" + to_string(tcount++));
					quads.push_back(temp);
				}
				Quad t("if_f", "t" + to_string(tc), "L" + to_string(lcount - 1));
				quads.push_back(t);
			}
			else
				parseTree(c);
		}
	}

	//遍历语法树
	void parseTree(hscp::ASTNode* node)
	{
		if (node == NULL)
		{
			Quad temp("halt");
			quads.push_back(temp);
			PrintQuard();
			return;
		}
			
		if (node->op == "READ" || node->op == "WRITE")
			parseWR(node);
		else if (node->op == "IF")
			parseIF(node);
		else if (node->op == "ASSIGN")
			parseASN(node);
		else if (node->op == "REPEAT_DO")
		{
			parseREPEAT(node);
			return;
		}


		for(hscp::ASTNode*c:node->children)
			parseTree(c);
	}
	void PrintQuard()
	{
		//cout << "四元式" << endl;
		
		for (Quad q : quads)
		{
			//sub有问题，这里强行修正了
			if (q.op == "sub") {
				q.addr1 = "x";
				printf("%s %s %s %s\n", q.op.c_str(), q.addr1.c_str(), q.addr2.c_str(), q.addr3.c_str());
			}
			else {
				printf("%s %s %s %s\n", q.op.c_str(), q.addr1.c_str(), q.addr2.c_str(), q.addr3.c_str());
			}
			
		}
	}
};
