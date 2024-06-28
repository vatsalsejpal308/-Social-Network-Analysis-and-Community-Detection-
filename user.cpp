#include<string>
using namespace std;

class User {
public:
    string name;
    int id;

    User(string name , int id) {
        this -> name = name;
        this -> id = id;
    }
};