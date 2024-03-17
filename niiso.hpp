#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <queue>
#include <map>

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <regex>
#include <cstdio>
#include <memory>
#include <stdexcept>

std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
	if (!pipe) throw std::runtime_error("popen() failed in exec()");
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	return result;
}

namespace sockchat {
	std::vector<std::string> segment(const std::string& input, const char delim) {
		std::vector<std::string> parts;
		std::istringstream iss(input);
		std::string part;
		char prev = '\0';

		while (std::getline(iss, part, delim)) {
			if (!(part.empty() && prev == delim)) {
				parts.push_back(part);
			}
			prev = delim;
		}

		return parts;
	}
};

class Niiso {
	private:
		std::string uid, auth;
		std::queue<std::string> q[2];
		std::mutex q_mtx;

		struct Command {
			std::string name;
			std::string desc;
			std::function<void(const std::vector<std::string>&)> handler;
		};

		struct Config {
			std::unordered_map<std::string, int> chances;
			std::vector<std::string> admin_ids;
			std::string user_id;
			std::string auth_key;

			void from_json(const json& j) {
				j.at("chances").get_to(chances);
				j.at("admin_ids").get_to(admin_ids);
				j.at("user_id").get_to(user_id);
				j.at("auth_key").get_to(auth_key);
			}
		};

		Config config;
		std::map<std::string, Command> cmds;
		std::map<std::string, std::string> uids;
		std::map<std::string, std::vector<std::string>> lines;
		std::vector<std::string> emotes;

		void _send(std::string msg) {
			q[1].push("2\t"+uid+"\t"+msg);
		}

		std::string _rand_line(const std::vector<std::string>& lines) {
			return lines[rand()%lines.size()];
		}

		std::string _rand_emote() {
			return emotes[rand()%emotes.size()];
		}

		void run_cmd(std::vector<std::string> args) {
			if (!cmds.count(args[0])) {
				std::cout << "Invalid command used." << std::endl;
				return;
			}
			cmds[args[0]].handler(args);
		}

		void embed_twitter(std::string msg) {
			std::regex post_regex("https://twitter.com/[a-zA-Z0-9_]+/status/[0-9]+");
			std::smatch match;
			std::vector<std::string> urls;
			while (std::regex_search(msg, match, post_regex)) {
				urls.push_back(match[0]);
				msg = match.suffix().str();
			}
			if (urls.size() > 0) {
				std::string msg_out("");
				for (const auto& url : urls)
					msg_out += exec((std::string("python3 eckser/main.py ") + url).c_str());
				send(msg_out);
			}
		}

		bool we_ball(int in) {
			return (rand()%in == 0);
		}

		void load_config(std::string fname) {
			std::ifstream file(fname);
			if (file.is_open()) {
				json data = json::parse(file);
				config.from_json(data);
				file.close();
			}
			else {
				json data;
				data["chances"] = {
					{ "timeout", 9 },
					{ "kick", 9 },
					{ "leave", 9 },
					{ "flood", 9 },
					{ "unauth", 9 },
					{ "args_bad", 9 },
					{ "args_evil", 9 },
					{ "format", 9 },
					{ "engie_one", 9 }
				};
				data["user_id"] = "USER_ID_STRING";
				data["auth_key"] = "AUTH_KEY_STRING";
				data["admin_ids"] = { "SOME_UID", "ANOTHER_UID" };
				std::ofstream out_file(fname);
				out_file << std::setw(4) << data << std::endl;
				out_file.close();
				load_config(fname);
			}
		}

	public:
		Niiso(std::string uid, std::string auth) {
			load_config("niiso_config.json");
			this->uid = config.user_id;
			this->auth = config.auth_key;
			srand(time(NULL));

			emotes = {
				":sob:",
				":random:",
				":evil:",
				":sunglasses:",
				":pensive:",
				":woomy:",
				":random-pain:",
				":pien:",
				"xd",
				"o-o"
			};

			lines["timeout"] = {
				"uh oh",
				"F interweb",
				"farewell",
				"bro why am i lagging",
				"no mom please dont turn off my router plea",
				"how",
				"flasher should restart server",
				"glad im hosted elsewhere",
				"rip sister",
				"mom no",
				"pingn't",
				"who died",
				"godspeed, miss dialup"
			};

			lines["kick"] = {
				"admin abuse",
				"that's not nice",
				"why",
				"battlefield 4 experience",
				"i got kicked once because someone made me spam a pyramid with echo",
				"i got kicked in the past i think",
				"what",
				"what's the crime",
				"power hungry",
				"huh",
				"damn",
				"what happened",
				"take cover"
			};

			lines["leave"] = {
				"see u",
				"gn",
				"mata ne",
				"ja ne",
				"see ya l8r",
				"cya",
				"aw",
				"catch u later",
				"baibaiiiii",
				"nighties",
				"i hope it's brb"
			};

			lines["flood"] = {
				"too many",
				"too much letters",
				"spambuster activate",
				"thanks satori",
				"good job",
				"thanks protector of the realm",
				"as long as it doesnt cover the entire chat",
				"nice",
				"it wasnt that much i think",
				"meditation time"
			};

			lines["unauth"] = {
				"no",
				"why would i",
				"i refuse",
				"fuck no",
				"nah",
				"nuh uh",
				"nope",
				"don't even try",
				"who are you to dictate that",
				"yeahhh no",
				"try contacting the owner",
				"you tried"
			};

			lines["args_bad"] = {
				"wrong args dummy",
				"this won't work",
				"i don't.. get it",
				"i don't get it",
				"i dont get it",
				"huh",
				"what",
				"that's not how it works",
				"idk that way to do this",
				"idk how to do that lol"
			};

			lines["args_evil"] = {
				"not even trying that",
				"im not trying that",
				"ye nah",
				"are you trying to kill me",
				"you're funny, funny guy",
				"im not gonna do that",
				"dont even try",
				"stop",
				"why",
				"would you download a zip bomb"
			};

			lines["format"] = {
				"wrong format, silly",
				"u mistyped something",
				"i think im dyslexic",
				"i might be dyslexic",
				"i cant read",
				"i cant parse scottish",
				"yeah coming right up (i have no idea what that says)"
			};

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
					if (we_ball(config.chances["unauth"])) {
						std::string message(_rand_line(lines["unauth"]));
						if (we_ball(3)) message += "?";
						if (we_ball(3)) message += (" " + _rand_emote());
						_send(message);
					}
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
						if(we_ball(config.chances["args_bad"])) {
							std::string message(_rand_line(lines["args_bad"]));
							if (we_ball(3)) message += (" " + _rand_emote());
							_send(message);
						}
						return;
					}
					try {
						std::vector<std::string> dice = sockchat::segment(args[1], 'd');
						if (dice.size() != 2) {
							if(we_ball(config.chances["args_bad"])) {
								std::string message(_rand_line(lines["args_bad"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
							return;
						}
						int nums[2];
						try {
							nums[0] = std::stoi(dice[0]);
							nums[1] = std::stoi(dice[1]);
						}
						catch (const std::exception&) {
							if(we_ball(config.chances["args_bad"])) {
								std::string message(_rand_line(lines["args_bad"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
							return;
						}
						if (nums[0] > 1000) {
							if(we_ball(config.chances["args_evil"])) {
								std::string message(_rand_line(lines["args_evil"]));
								if (we_ball(3)) message += "?";
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
							return;
						}
						if (nums[0] < 1 || nums[1] < 1) {
							if(we_ball(config.chances["args_evil"])) {
								std::string message(_rand_line(lines["args_evil"]));
								if (we_ball(3)) message += "?";
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
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
						if(we_ball(config.chances["format"])) {
							std::string message(_rand_line(lines["format"]));
							if (we_ball(3)) message += (" " + _rand_emote());
							_send(message);
						}
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
						if (args[a].at(0) == '!' || args[a].at(0) == '^') {
							if(we_ball(config.chances["args_evil"])) {
								std::string message(_rand_line(lines["args_evil"]));
								if (we_ball(3)) message += "?";
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
							return;
						}
						msg += (args[a]+" ");
					}
					_send(msg);
				}
			};

			cmds["^steam"] = {
				"steam",
				"generates a random steam key :evil:",
				[this](const std::vector<std::string>& args) {
					std::string symbols = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890";
					int segments = 3+rand()%6;
					std::string msg("");
					for (int s=0; s<segments; s++) {
						for (int c=0; c<5; c++) { msg += symbols[rand()%symbols.size()]; }
						if (s != segments-1) msg += "-";
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
			std::string out;
			std::vector<std::string> parts;
			std::vector<std::string> args;
			while (!q[0].empty()) {
				packet = q[0].front(); q[0].pop();
				parts = sockchat::segment(packet, '\t');
				switch(std::stoi(parts[0])) {
					// TODO: rest of the packets lol
					case 1:
						out = "";
						if ((parts[1] != "y" && parts[1] != "n") && we_ball(10)) {
							uids[parts[3]] = parts[4];
							if (we_ball(8)) out += "helo ";
							out += parts[3];
							if (we_ball(6)) out += "er";
							_send(out);
						}
						break;
					case 2:
						std::cout << (uids[parts[2]] + ": " + parts[3] + "\n");
						args = sockchat::segment(parts[3], ' ');
						if (args[0].at(0) == '^') run_cmd(args);
						if (parts[2] == "186" && parts[3].find(": 1.") != std::string::npos && we_ball(config.chances["engie_one"])) run_cmd({"^one"});
						embed_twitter(parts[3]);
						break;
					case 3:
						if (parts[3] == "kick") {
							if(we_ball(config.chances["kick"])) {
								std::string message(_rand_line(lines["kick"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
						}
						if (parts[3] == "timeout") {
							if(we_ball(config.chances["timeout"])) {
								std::string message(_rand_line(lines["timeout"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
						}
						if (parts[3] == "flood") {
							if(we_ball(config.chances["flood"])) {
								std::string message(_rand_line(lines["flood"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
						}
						if (parts[3] == "leave") {
							if(we_ball(config.chances["leave"])) {
								std::string message(_rand_line(lines["leave"]));
								if (we_ball(3)) message += (" " + _rand_emote());
								_send(message);
							}
						}
						break;
					case 7:
						switch(std::stoi(parts[1])) {
							case 0:
								for (int i=0; i<std::stoi(parts[2]); i++) {
									uids[parts[3+i*5]] = parts[4+i*5];
								}
								break;
							case 1:
								std::cout << (parts[4] + ": " + parts[7] + "\n");
								break;
							case 2:
								break;
							default:
								break;
						}
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
