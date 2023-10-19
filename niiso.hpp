#include <iostream>
#include <mutex>
#include <string>
#include <queue>
#include <map>

namespace sockchat {
	std::vector<std::string> segment(const std::string& input, const char delim) {
		std::vector<std::string> parts;
		std::istringstream iss(input);
		std::string part;

		while (std::getline(iss, part, delim)) {
			parts.push_back(part);
		}

		return parts;
	}
};

class Niiso {
	private:
		std::string uri, uid, auth;
		std::queue<std::string> q[2];
		std::mutex q_mtx;

		struct Command {
			std::string name;
			std::string desc;
			std::function<void(const std::vector<std::string>&)> handler;
		};

		std::map<std::string, Command> cmds;

		void _send(std::string msg) {
			q[1].push("2\t"+uid+"\t"+msg);
		}

		void run_cmd(std::vector<std::string> args) {
			if (!cmds.count(args[0])) {
				std::cout << "Invalid command used." << std::endl;
				return;
			}
			cmds[args[0]].handler(args);
		}

	public:
		Niiso(std::string uri, std::string uid, std::string auth) {
			this->uri = uri;
			this->uid = uid;
			this->auth = auth;
			srand(time(NULL));

			cmds["^help"] = {
				"help",
				"displays available commands",
				[this](const std::vector<std::string>& args) {
					std::string msg("");
					msg += "[b]available ^commands:[/b]\n";
					for (const auto& pair : cmds) {
						msg += ("  [color=#f45dcc]"+pair.second.name+"[/color] - "+pair.second.desc+"\n");
					}
					_send(msg);
				}
			};

			cmds["^exit"] = {
				"exit",
				"exits the chat",
				[this](const std::vector<std::string>& args) {
					_send("lmao you thought");
				}
			};

			cmds["^one"] = {
				"one",
				"On the bright side... 1",
				[this](const std::vector<std::string>& args) {
					_send("[img]https://saikuru.net/engie_one.png[/img]");
				}
			};

			cmds["^roll"] = {
				"roll",
				"rolls the dice in your beloved format (ex. 1d6)",
				[this](const std::vector<std::string>& args) {
					if (args.size() != 2) {
						_send("wrong args, dummy");
						return;
					}
					try {
						std::vector<std::string> dice = sockchat::segment(args[1], 'd');
						if (dice.size() != 2) {
							_send("how many 'd's did you use");
							return;
						}
						int nums[2] = { std::stoi(dice[0]), std::stoi(dice[1]) };
						if (nums[0] < 1 || nums[1] < 1) {
							_send("how am i supposed to roll that");
							return;
						}
						std::string msg("");
						msg += std::to_string(1+rand()%nums[1]);
						for (int d=1; d<nums[0]; d++) {
							msg += (", " + std::to_string(1+rand()%nums[1]));
						}
						_send(msg);
					}
					catch (const std::exception&) {
						_send("wrong format, silly");
						return;
					}
				}
			};

			cmds["^echo"] = {
				"echo",
				"repeats your message",
				[this](const std::vector<std::string>& args) {
					std::string msg("");
					for (int a=1; a<(int)args.size(); a++) {
						msg += (args[a]+" ");
					}
					_send(msg);
				}
			};
		}

		void join() {
			q_mtx.lock();
			q[1].push("1\tMisuzu\t"+auth);
			q_mtx.unlock();
		}

		void send(std::string msg) {
			q_mtx.lock();
			_send(msg);
			q_mtx.unlock();
		}

		void ping() {
			q_mtx.lock();
			q[1].push("0\t"+uid);
			q_mtx.unlock();
		}

		void add(std::string packet) {
			q_mtx.lock();
			q[0].push(packet);
			q_mtx.unlock();
		}

		void serve() {
			q_mtx.lock();
			std::string packet;
			std::vector<std::string> parts;
			std::vector<std::string> args;
			while (!q[0].empty()) {
				packet = q[0].front(); q[0].pop();
				parts = sockchat::segment(packet, '\t');
				switch(std::stoi(parts[0])) {
					// TODO: rest of the packets lol
					case 2:
						std::cout << (parts[2] + ": " + parts[3] + "\n");
						args = sockchat::segment(parts[3], ' ');
						if (args[0].at(0) == '^') run_cmd(args);
						if (parts[2] == "186" && parts[3].find(": 1.") != std::string::npos) run_cmd({"^one"});
						break;
					default:
						break;
				}
			}
			q_mtx.unlock();
		}

		std::string next_msg() {
			std::string msg("");
			q_mtx.lock();
			if (!q[1].empty()) {
				msg = q[1].front(); q[1].pop();
			}
			q_mtx.unlock();
			return msg;
		}
};
