#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
using namespace std;

//Node struct, used to track user-defined variables
struct Node {
	string var;
	int val;

	Node* next;
	Node(string name, int num) : var(name), val(num), next(NULL) {  }
};

//ScopeNode struct, used to track which variables are created in a scope
struct ScopeNode {
	string var;
	ScopeNode* next;

	ScopeNode(string name) : var(name), next(NULL) {  }
};

//add function to create a linked list of ScopeNodes
ScopeNode* addScopeNode(ScopeNode* head, string key) {
	ScopeNode* newNode = new ScopeNode(key);
	if (head == NULL)
		head = newNode;
	else {
		newNode->next = head;
		head = newNode;
	}
	return head;
}

//Hash table, stores all the user-defined variables
class HashTable {
private:
	int size;
	Node** table;

public:
	//constructors
	HashTable() : size(25), table(new Node*[25]) {
		for (int i = 0; i < size; i++)
			table[i] = NULL;
	}
	HashTable(int num) : size(num), table(new Node*[size]) {
		for (int i = 0; i < size; i++)
			table[i] = NULL;
	}

	//hash function
	int hash(string str) {
		int sum = 0;
		for (int i = 0; i < str.size(); i++)
			sum += str[i] * (i + 1);
		return sum % size;
	}

	/*add a value to the table
	  use chaining for collision resolution*/
	void add(string key, int val) {
		int index = hash(key);

		Node* newNode = new Node(key, val);
		if (table[index] == NULL)
			table[index] = newNode;
		else {
			newNode->next = table[index];
			table[index] = newNode;
		}
	}

	void edit(string key, int v) {
		int index = hash(key);
		if (table[index] != NULL) {
			Node* temp = table[index];
			while (temp->next != NULL) {
				if (temp->var == key) {
					temp->val = v;
					break;
				}
				temp = temp->next;
			}
			if (temp->var == key)
				temp->val = v;
		}
	}

	//lookup a value using the variable name
	int lookup(string key) {
		int index = hash(key);
		if (table[index] == NULL)
			return -1; //not found
		else {
			Node* temp = table[index];
			while (temp->next != NULL) {
				if (temp->var == key)
					return temp->val;
				temp = temp->next;
			}
			if (temp->var == key)
				return temp->val;
			return -1;
		}
	}

	//remove a variable from the table using the variable name
	void remove(string key) {
		int index = hash(key);
		if (table[index] == NULL)
			return;
		else if (table[index]->var == key)
			table[index] = table[index]->next;
		else {
			Node* temp = table[index];
			while (temp->next != NULL && temp->next->next != NULL) {
				if (temp->next->var == key) {
					temp->next = temp->next->next;
					return;
				}
				temp = temp->next;
			}
			if (temp->next != NULL && temp->next->var == key)
				temp->next = temp->next->next;
		}
	}

	//delete all the variables within a scope
	void removeScope(ScopeNode* head) {
		while (head->next != NULL) {
			remove(head->var);
			head = head->next;
		}
		remove(head->var);
	}
};

/*isInt, a function that takes in a string
  returns true if the string is a number; otherwise, false */
bool isInt(string s) {
	for (int i = 0; i < s.length(); i++)
		if (!isdigit(s[i]))
			return false;
	return true;
}

/*isOperator, a function that takes in a string
  returns true if the string is a math operator; otherwise, false */
bool isOperator(string s) {
	if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "^")
		return true;
	return false;
}

/*isUnaryOperator, a function that takes in a string
  returns true if the string is a unary operator (++, --); otherwise, false */
bool isUnaryOperator(string s) {
	if (s == "++" || s == "--")
		return true;
	return false;
}

/*doOperation, a function that takes a hashtable and an expression of form a _ b
  returns the result after the operation is performed */
int doOperation(HashTable h, string num1, string operation, string num2) {
	//get the numbers being operated on
	int a, b;
	if (isInt(num1))
		a = stoi(num1);
	else {
		a = h.lookup(num1);
		if (a == -1) {
			cout << num1 << " IS UNDEFINED" << endl;
			return INT_MIN;
		}
	}
	if (isInt(num2))
		b = stoi(num2);
	else {
		b = h.lookup(num2);
		if (b == -1) {
			cout << num2 << " IS UNDEFINED" << endl;
			return INT_MIN;
		}
	}
	//do the operation
	switch (operation[0]) {
	case '+':
		return a + b;
		break;
	case '-':
		return a - b;
		break;
	case '*':
		return a * b;
		break;
	case '/':
		return 1.0 * a / b;
		break;
	case '%':
		return a % b;
		break;
	case '^':
		return pow(a, b);
		break;
	}
}

int main() {
	//create vars to track the scope
	ScopeNode** scopesList = new ScopeNode* [10];
	for (int i = 0; i < 10; i++)
		scopesList[i] = NULL;
	int scopeCount = 0;

	//create hashtable for the variables
	HashTable vars = HashTable(10);

	//variables for processing input into commands
	string input;
	string* processed_input = new string[20];
	int pos, counter;

	//get input
	while (true) {
		getline(cin, input);
		counter = 0;
		
		//remove leading whitespace
		while (input[0] == ' ')
			input.erase(0, 1);

		//split input into words using " " as delimiter
		while ((pos = input.find(" ")) != string::npos) {
			processed_input[counter] = input.substr(0, pos);
			input.erase(0, pos + 1);
			counter++;
		}
		processed_input[counter] = input;
		counter++;

		//check for valid commands

		/*COM [text]
		  a comment. entire line is ignored by compiler.*/
		if (processed_input[0].compare("COM") == 0)
			continue;

		/*START
		  begin a new scope*/
		else if (processed_input[0].compare("START") == 0) {
			if (scopeCount < 9)
				scopeCount++;
		}

		/*FINISH
		  close the current scope*/
		else if (processed_input[0].compare("FINISH") == 0) {
			ScopeNode* temp = scopesList[scopeCount];
			while (temp != NULL && temp->next != NULL) {
				vars.remove(temp->var);
				temp = temp->next;
			}
			if (temp != NULL)
				vars.remove(temp->var);
			scopesList[scopeCount] = NULL;
			if (scopeCount > 0)
				scopeCount--;
		}

		/*VAR [var] = [expression]
		  defines a variable and adds to hashtable*/
		else if (processed_input[0].compare("VAR") == 0) {
			// VAR [var] = [num]
			if (counter == 4 && isInt(processed_input[3])) {
				vars.add(processed_input[1], stoi(processed_input[3]));
				scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
			}
			// VAR [var] = [another var]
			else if (counter == 4 && !isInt(processed_input[3])) {
				int res = vars.lookup(processed_input[3]);
				if (res != -1) {
					vars.add(processed_input[1], res);
					scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
				}
				else
					cout << "Error defining variable " << processed_input[1] << endl;
			}
			// VAR [var] = ++||-- [num]||[var]
			else if (counter == 5 && isUnaryOperator(processed_input[3])) {
				if (isInt(processed_input[4])) {
					if (processed_input[3] == "++")
						vars.add(processed_input[1], stoi(processed_input[4]) + 1);
					else
						vars.add(processed_input[1], stoi(processed_input[4]) - 1);
					scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
				}
				else {
					int res = vars.lookup(processed_input[4]);
					if (res != -1) {
						if (processed_input[3] == "++")
							vars.add(processed_input[1], res + 1);
						else
							vars.add(processed_input[1], res - 1);
						scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
					}
					else
						cout << "Error defining variable " << processed_input[1] << endl;
				}
			}
			// VAR [var] = [num]||[var] ++||--
			else if (counter == 5 && isUnaryOperator(processed_input[4])) {
				if (isInt(processed_input[3])) {
					vars.add(processed_input[1], stoi(processed_input[3]));
					scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
				}
				else {
					int res = vars.lookup(processed_input[3]);
					if (res != -1) {
						vars.add(processed_input[1], res);
						if (processed_input[4] == "++")
							vars.edit(processed_input[3], res + 1);
						else
							vars.edit(processed_input[3], res - 1);
						scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
					}
					else
						cout << "Error defining variable " << processed_input[1] << endl;
				}
			}
			// VAR [var] = [num]||[var] [operator] [num]||[var]
			else if (counter == 6) {
				int res = doOperation(vars, processed_input[3], processed_input[4], processed_input[5]);
				if (res != INT_MIN) {
					vars.add(processed_input[1], res);
					scopesList[scopeCount] = addScopeNode(scopesList[scopeCount], processed_input[1]);
				} 
				else
					cout << "Error defining variable " << processed_input[1] << endl;;
			}
			else
				cout << "Error defining variable " << processed_input[1] << endl;
		}

		/*PRINT [expression]
		  outputs expression's result to console*/
		else if (processed_input[0].compare("PRINT") == 0) {
			// PRINT [num]||[var]
			if (counter == 2) {
				if (isInt(processed_input[1]))
					cout << processed_input[1] << endl;
				else {
					int res = vars.lookup(processed_input[1]);
					if (res == -1)
						cout << processed_input[1] << " IS UNDEFINED" << endl;
					else
						cout << processed_input[1] << " IS " << res << endl;
				}
			}
			else if (counter == 3) {
				// PRINT ++||-- [num]||[var]
				if (isUnaryOperator(processed_input[1])) {
					if (isInt(processed_input[2])) {
						if (processed_input[1] == "++")
							cout << processed_input[1] << " " << processed_input[2] << " IS " << stoi(processed_input[1]) + 1 << endl;
						else
							cout << processed_input[1] << " " << processed_input[2] << " IS " << stoi(processed_input[1]) - 1 << endl;
					}
					else {
						int res = vars.lookup(processed_input[2]);
						if (res != -1) {
							if (processed_input[1] == "++") {
								vars.edit(processed_input[2], res + 1);
								cout << processed_input[1] << " " << processed_input[2] << " IS " << res + 1 << endl;
							}
							else {
								vars.edit(processed_input[2], res - 1);
								cout << processed_input[1] << " " << processed_input[2] << " IS " << res - 1 << endl;
							}							
						}
						else
							cout << processed_input[2] << " IS UNDEFINED" << endl;
					}
				}
				// PRINT [num]||[var] ++||-- 
				else if (isUnaryOperator(processed_input[2])) {
					if (isInt(processed_input[1]))
						cout << processed_input[1] << " " << processed_input[2] << " IS " << processed_input[2] << endl;
					else {
						int res = vars.lookup(processed_input[1]);
						if (res != -1) {
							cout << processed_input[1] << " " << processed_input[2] << " IS " << res << endl;
							if (processed_input[2] == "++")
								vars.edit(processed_input[1], res + 1);
							else
								vars.edit(processed_input[1], res - 1);
						}
						else
							cout << processed_input[1] << " IS UNDEFINED" << endl;
					}
				}
				else
					cout << "Invalid expression" << endl;
			}
			// PRINT [expression]
			else {
				if (!isOperator(processed_input[2]))
					cout << "Invalid expression" << endl;
				else {
					int res = doOperation(vars, processed_input[1], processed_input[2], processed_input[3]);
					if (res != INT_MIN)
						cout << processed_input[1] << " " << processed_input[2] << " " << processed_input[3] << " IS " << res << endl;
				}
			}
		}

		/*if not a keyword, treat it as an assignment expression
		  look up the variable and reassign it to the expression's value*/
		else if (vars.lookup(processed_input[0]) != -1) {
			// [var] ++||--
			if (counter == 2 && isUnaryOperator(processed_input[1])) {
				if (processed_input[1] == "++")
					vars.edit(processed_input[0], vars.lookup(processed_input[0]) + 1);
				else
					vars.edit(processed_input[0], vars.lookup(processed_input[0]) - 1);
			}
			// [var] = [num]
			else if (counter == 3 && isInt(processed_input[2]))
				vars.edit(processed_input[0], stoi(processed_input[2]));
			// [var] = [another var]
			else if (counter == 3 && !isInt(processed_input[2])) {
				int res = vars.lookup(processed_input[2]);
				if (res != -1)
					vars.edit(processed_input[0], res);
				else
					cout << "Error redefining variable " << processed_input[0] << endl;
			}
			// [var] = ++||-- [num]||[var]
			else if (counter == 4 && isUnaryOperator(processed_input[2])) {
				if (isInt(processed_input[3])) {
					if (processed_input[2] == "++")
						vars.edit(processed_input[0], stoi(processed_input[3]) + 1);
					else
						vars.edit(processed_input[0], stoi(processed_input[3]) - 1);
				}
				else {
					int res = vars.lookup(processed_input[3]);
					if (res != -1) {
						if (processed_input[2] == "++")
							vars.add(processed_input[0], res + 1);
						else
							vars.add(processed_input[0], res - 1);
					}
					else
						cout << "Error redefining variable " << processed_input[0] << endl;
				}
			}
			// [var] = [num]||[var] ++||--
			else if (counter == 4 && isUnaryOperator(processed_input[3])) {
				if (isInt(processed_input[2]))
					vars.edit(processed_input[0], stoi(processed_input[2]));
				else {
					int res = vars.lookup(processed_input[2]);
					if (res != -1) {
						vars.add(processed_input[0], res);
						if (processed_input[3] == "++")
							vars.edit(processed_input[2], res + 1);
						else
							vars.edit(processed_input[2], res - 1);
					}
					else
						cout << "Error redefining variable " << processed_input[0] << endl;
				}
			}
			// [var] = [num]||[var] [operator] [num]||[var]
			else if (counter == 5) {
				int res = doOperation(vars, processed_input[2], processed_input[3], processed_input[4]);
				if (res != INT_MIN)
					vars.add(processed_input[0], res);
				else
					cout << "Error redefining variable " << processed_input[0] << endl;;
			}
			else
				cout << "Error redefining variable " << processed_input[0] << endl;
		}

		else if (processed_input[0] == "EXIT")
			break;
	}
}

/* Sample Outputs

 * Program provided on Canvas: 
	BORAMIR IS 25
	LEGOLAS IS 101
	GANDALF IS UNDEFINED
	BORAMIR * 2 IS 52
	GANDALF IS 49
	BORAMIR IS 26
	GANDALF IS UNDEFINED
	LEGOLAS IS 1000
	LEGOLAS IS 1000
	LEGOLAS IS 999

 * My BORG program, source code on Canvas:
	APPLE IS 33
	PEN IS 32
	PINEAPPLE IS 45
	++ PINEAPPLE IS 46
	5 * 10 IS 50
	12 % 10 IS 2
	PINEAPPLE - 10 IS 36
	PINEAPPLE IS UNDEFINED
	APPLE IS 33
	APPLE + 54 IS 87
	PEN IS 31
 */
