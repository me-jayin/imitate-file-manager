using namespace std;

/**
 * 定义命令类 
 */
class Command {
	private:
		string cmd; // 用户输入的命令 
	public:
		string args[5]; // 进行解析后的参数 
		int argsCount; // 参数个数 
		User *user; // 当前用户 
		FileManager *fm; // 当前文件管理器 
		Command(User *user, FileManager *fm) {
			// 如果无指定文件管理器则直接退出 
			if (fm == NULL) {
				exit(1);
			}
			this->user = user;
			this->fm = fm;
		};
		int analyse(string cmd);
		void command();
};

class Handlers {
	public:
		static void exitc(Command* cmd) {
			cout << "系统退出中...";
			exit(1);	
		};
		static void dir(Command* cmd) {
			cmd->fm->displayChildDirectory(cmd->user);
		};
		static void clear(Command* cmd) {
	        system("cls");
		}
		static void mkdir(Command* cmd) {
			if (cmd->argsCount != 1) {
				cout << "参数错误，参数列表[dir_name]" << endl;
				return;
			}
			// 调用创建文件
			cmd->fm->createDirectory(cmd->user, cmd->args[0]);
		}
		static void cd(Command* cmd) {
			if (cmd->argsCount != 1) {
				cout << "参数错误，参数列表[dir_name]" << endl;
				return;
			} 
			
			cmd->fm->cd(cmd->user, cmd->args[0]);
		}
		static void create(Command* cmd) {
			if (cmd->argsCount != 1) {
				cout << "参数错误，参数列表[file_name]" << endl;
				return;
			} 
			
			cmd->fm->createFile(cmd->user, cmd->args[0]);
		}
		static void open(Command* cmd) {
			if (cmd->argsCount != 1) {
				cout << "参数错误，参数列表[file_name]" << endl;
				return;
			}
			
			cmd->fm->openFile(cmd->user, cmd->args[0]);
		}
		static void rm(Command* cmd) {
			if (cmd->argsCount != 1) {
				cout << "参数错误，参数列表[file_name]" << endl;
				return;
			}
			
			cmd->fm->rm(cmd->user, cmd->args[0]);
		}
		static void prtbit(Command* cmd) {
			cmd->fm->bitMap->display(); 
		}
		static void rfm(Command* cmd) {
			FileManager *fm = initFileManager(true);
			if (fm == NULL) return;
			cmd->fm = fm;
		}
		static void logout(Command* cmd) {
			Handlers::clear(cmd);
			cmd->user = loginForm();
			cmd->fm->cd(cmd->user, "/");
			Handlers::clear(cmd);
			Handlers::help(cmd);
			getchar();
		}
		static void help(Command* cmd) {
			cout << 
	"               何妨文件系统\n\
	命令     命令描述            语法\n\
	exit     退出系统            exit\n\
	clear    清除屏幕            clear\n\
	help     显示帮助菜单        help\n\
	cd       前往指定文件夹      cd [dir_name]\n\
	dir      显示文件列表        dir\n\
	mkdir    创建文件夹          mkdir [dir_name]\n\
	create   创建文件            create [file_name]\n\
	open     打开文件            open [file_name]\n\
	rm       删除文件或文件夹    rm [dir_name | file_name]\n\
	prtbit   打印位示图          prtbit\n\
	logout   注销用户            logout\n\
	rfm      格式化系统          rfm\n";
		};
};

/** 系统命令 */
const char* syscmds[] = {
    "exit", "help", "dir", "clear", "mkdir", 
	"cd", "create", "open", "rm", "prtbit", "logout", "rfm",  
};
/** 对应命令的处理方法 */
void (*handlers[])(Command*) = {
    Handlers::exitc, Handlers::help, Handlers::dir, Handlers::clear, Handlers::mkdir,
    Handlers::cd, Handlers::create, Handlers::open, Handlers::rm, Handlers::prtbit, Handlers::logout, Handlers::rfm
	 
};
/** 命令个数 */
int cmdsize = sizeof(syscmds) / sizeof(char*);

/**
 * 解析命令 
 */
int Command::analyse(string cmd) {
	int i = 0;
	for (; i < cmdsize; i++) {
		if (strStartWith(cmd.c_str(), syscmds[i])) {
			break;
		}
	}
	if (i == cmdsize) {
		return -1;
	}
	
	// 解析参数等操作 
	argsCount = 0;
	bool flag = false;
	int len = cmd.length(), cmdlen = strlen(syscmds[i]), start = 0, end = 0;
	const char *cmdcrs = cmd.c_str();
	for (int j = cmdlen; j < len; j++) {
		if (argsCount == 5) break;
		else if (cmdcrs[j] == ' ' && !flag) {
			start = end = j;
			flag = true;
		} else if (cmdcrs[j] == ' ' && start == end) {
			start++;
			end++;
		} else if (cmdcrs[j] == ' ') {
			// 找到了下一个空格
			args[argsCount++] = cmd.substr(start + 1, j - start - 1);
			cout << args[argsCount-1] << endl;
			start = end = j;
		} else {
			end++;
		}
	}
	if (argsCount != 5 && start != end) {
		args[argsCount++] = cmd.substr(start + 1, len);
	}
	return i;
}
/**
 * 开始执行命令 
 */
void Command::command() {
	Handlers::clear(NULL);
	Handlers::help(NULL);
	cin.get();
	while (true) {
		cout << "[" << user->usr << "@ " << this->fm->getCurrentDirectoryName() << "]# ";
		
		getline(cin, cmd);
		int idx = this->analyse(cmd);
		if (idx < 0) {
			if (!strStartWith(cmd.c_str(), "\n") && strlen(cmd.c_str()) != 0) cout << "无效的命令" << endl;
			continue;
		}
		handlers[idx](this);
	}
} 

