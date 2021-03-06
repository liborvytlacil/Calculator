#pragma once
#include<string>
#include<vector>


using namespace std;

class Variable {
public:
	string name;
	double value;
};

class VarTable {
private:
	vector<Variable> variables;

	Variable* getVariable(string name) {
		for (Variable& var : variables) {
			if (var.name == name) {
				return &var;
			}
		}

		return nullptr;
	}

public:

	double get(string name) {
		for (const Variable& var : variables) {
			if (name == var.name) {
				return var.value;
			}
		}

		throw runtime_error("Undefined variable '" + name + "'");
	}

	double define(string name, double value) {
		Variable *var = getVariable(name);
		if (var == nullptr) {
			variables.push_back(Variable{ name, value });
		}
		else {
			var->value = value;
		}

		return value;
	}
};