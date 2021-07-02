#include<stdlib.h>
#include <conio.h>
#include<iostream>
#include<fstream>
#include<string.h>
using namespace std;

const char* USER_FILE = "user.dat";
class User {
	public:
		char usr[10]; // 用户名 
		char pwd[10]; // 密码 
		User *next;
		User() {};
		User(char usr[10], char pwd[10]) {
			strcpy(this->usr, usr);
			strcpy(this->pwd, pwd);
			this->next = NULL; 
		};
		bool login(char *usr, char *pwd) {
			return this->currentUser(usr) && strcmp(this->pwd, pwd) == 0;
		};
		bool currentUser(char *usr) {
			return strcmp(this->usr, usr) == 0;
		}
};

User* loginForm();
/** 释放用户对象 */
void releaseUser(User* user);
/** 加载用户信息 */
User* loadUser();
void saveUser(User *user);
/** 注册 */
void regist();
/** 登录 */
User* login();

void inputPwd(char *p) {
	while(*p=getch()) {
		if(*p == 0x0d) {
			*p = '\0'; //将输入的回车键转换成空格
			break;
		}
		printf("*");    //将输入的密码以"*"号显示
		p++;
	}
	cout << endl;
}

User* loginForm() {
	char choice, temp[10];
	User *current = NULL;
	cout << "       系统登录" << endl;
	cout << "    注册(R)" << endl;
	cout << "    用户(L)" << endl;
	do {
		cout << "> ";
		cin >> choice;
		gets(temp);
		if ((choice == 'R') || (choice == 'r')) {
			regist();
		} else if ((choice == 'L') || (choice == 'l')){
			current = login();
		} else {
			cout << "选择错误，请重新选择！" << endl;
		}
	} while (current == NULL);
	return current;
}

User* login() {
	char usr[10], pwd[10];
	cout << "请输入用户名：";
	cin >> usr;
	cout << "请输入密码：";
	inputPwd(pwd);
	
	User *user = loadUser(), *temp = user;
	bool flag = false;
	while (temp != NULL) {
		if (temp->login(usr, pwd)) {
			flag = true;
			break;
		}
		temp = temp->next;
	}
	releaseUser(user);
	if (!flag) {
		cout << "×登录失败，密码或用户名错误" << endl;
	} else {
		cout << "登录成功" << endl;
	}
	return flag ? new User(usr, pwd) : NULL;
}

void regist() {
	char usr[10], pwd[10], repwd[10];
	cout << "请输入用户名：";
	cin >> usr;
	cout << "请输入密码：";
	inputPwd(pwd);
	cout << "请确认密码：";
	inputPwd(repwd);
	
	if (strcmp(repwd, pwd) != 0) {
		cout << "输入的两次密码不一致，注册失败" << endl;
		return;
	}
	
	User *record = loadUser(), *temp = record;
	while (temp != NULL) {
		if (temp->currentUser(usr)) {
			cout << "创建失败，用户已存在" << endl;
			releaseUser(record);
			return;
		}
		temp = temp->next;
	}
	
	User *user = new User(usr, pwd);
	user->next = record;
	// 保存文件 
	saveUser(user);
	releaseUser(user);
	cout << "用户创建成功" << endl;
}

void releaseUser(User* user) {
	while (user != NULL) {
		User *temp = user;
		user = user->next;
		free(temp);
	}
}

User* loadUser() {
	User *_u = NULL, *temp = new User();
	ifstream inFile(USER_FILE, ios::in | ios::binary); //二进制读方式打开
	while(inFile.read((char*)temp, sizeof(User))) { //一直读到文件结束
		temp->next = _u;
		_u = temp;
		temp = new User();
    }
    inFile.close();
    return _u;
}

void saveUser(User *user) {
	ofstream outFile(USER_FILE, ios::out | ios::binary); //二进制写方式打开
	while (user != NULL) {
		outFile.write((char*) user, sizeof(User));
		user = user->next;
	}
	outFile.close();
}
