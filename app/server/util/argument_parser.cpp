//
// Created by diago on 4/18/2024.
//

#include "argument_parser.hpp"

#include <typeindex>

lurch::result<double> lurch::argument_parser::safe_to_double(const std::string token) {

	if (token.find('.') == std::string::npos) {
		return error("failed.");
	}

	double d = 0.00;

	try { d = std::stod(token); }
	catch ( const std::exception& e) { return error("failed."); }
	return d;
}

lurch::result<int64_t> lurch::argument_parser::safe_to_signed_integer(const std::string token) {

	int64_t i = 0;

	try { i = std::stoll(token); }
	catch (const std::exception& e) { return error("failed.");}
	return i;
}

lurch::result<bool> lurch::argument_parser::safe_to_boolean(const std::string token) {

	if (token == "true") { return true; }
	else if (token == "false") { return false;}
	return error("failed.");
}

lurch::argument_parameter lurch::argument_parser::infer_parameter_type(const std::string token) {

	result<int64_t> param_i64     =   argument_parser::safe_to_signed_integer(token);
	result<bool> param_bool       =   argument_parser::safe_to_boolean(token);
	result<double> param_double   =   argument_parser::safe_to_double(token);

	if (param_bool) {
		return param_bool.value();
	}

	else if (param_double) {
		return param_double.value();
	}

	else if (param_i64) {
		return param_i64.value();
	}

	return token; //default, return the string back
}


void lurch::argument_parser::strip_whitespace(std::string& str) {

	if (str.empty()) {
		return;
	}

	size_t first_non_space = str.find_first_not_of(" \t");
	if (first_non_space != std::string::npos) {
		str.erase(0, first_non_space);
	}

	if (str.empty())
		return;


	if (str.back() == ' ' || str.back() == '\t') {
		size_t r_first_non_space = str.size() - 1;
		while ((r_first_non_space - 1) >= 0 && (str[r_first_non_space - 1] == ' ' || str[r_first_non_space - 1] == '\t')) {
			--r_first_non_space;
		}

		str.erase(r_first_non_space);
	}
}


std::pair<std::string, std::vector<std::string>> lurch::argument_parser::tokenize_input(std::string& str) {

	std::istringstream        iss(str);
	std::string               name;
	std::string               token;
	std::vector<std::string>  tokens;


	std::getline(iss, name, ' ');
	while (std::getline(iss, token, ' ')) {
		tokens.emplace_back(token);
	}

	return std::make_pair(name, tokens);
}


std::pair<std::string, size_t> lurch::argument_parser::get_quoted_string(const std::vector<std::string>& tokens, size_t index) {

	if (index >= tokens.size() || tokens.empty()) {
		return {};
	}

	std::string quoted_string;
	size_t start = index;

	for (index; index < tokens.size(); index++) {
		std::string cur_token = tokens[index];
		quoted_string += (cur_token + ' ');

		if (!cur_token.empty() && cur_token.back() == '\"') {
			quoted_string.erase(quoted_string.size() - 1);
			break;
		}
	}

	if (quoted_string.empty() || quoted_string.back() != '\"') {
		return {};
	}

	return std::make_pair(quoted_string, (index - start) + 1);
}


lurch::result<lurch::command> lurch::argument_parser::parse(std::string raw) {

	lurch::command cmd;
	argument cur_argument = {.parameter = std::monostate()};

	strip_whitespace(raw);
	if (raw.empty()) {
		return error("empty input.");
	}

	auto [name, tokens] = tokenize_input(raw);
	if (name.empty()) {
		return error("empty input.");
	}

	for (size_t i = 0; i < tokens.size(); ++i) {

		std::string tok = tokens[i];
		size_t not_of_hyphen = tok.find_first_not_of('-');

		if (not_of_hyphen == 1 || not_of_hyphen == 2) {
			if (!cur_argument.flag_name.empty()) {
				cmd.arguments.push_back(cur_argument);
			}

			cur_argument.flag_name = tok;
		}

		else {
			if (cur_argument.flag_name.empty()) {
				return error("Invalid formatting.");
			}

			if (tok[0] == '\"') {
				auto [str, size] = get_quoted_string(tokens, i);

				if (str.empty() || !size) {
					cur_argument.parameter = tok;
				}

				else {
					cur_argument.parameter = str;
					i += size - 1;
				}
			}

			else {
				cur_argument.parameter = infer_parameter_type(tok);
			}

			cmd.arguments.push_back(cur_argument);
			cur_argument.flag_name.clear();
		}

		cur_argument.parameter = std::monostate();
	}

	if (!cur_argument.flag_name.empty()) {
		cmd.arguments.push_back(cur_argument);
	}

	cmd.name = name;
	return lurch::result<lurch::command>(cmd);
}

lurch::formatted_command& lurch::accepted_commands::add_command(const std::string& name) {
	return commands.emplace_back(formatted_command(name));
}

bool lurch::accepted_commands::matches(const lurch::command& passed) {
	for(const auto& command : this->commands) {
		if(command.name == passed.name) {
			return match_flags(passed, command);
		}
	}

	return false;
}

bool lurch::accepted_commands::match_flags(const lurch::command& passed, const formatted_command& to_compare) {

	std::map<std::string, std::optional<std::type_index>> arg_map;
	for(const auto&[flag_name, parameter] : passed.arguments) {
		if(arg_map.contains(flag_name)) {
			return false;
		}

		arg_map[flag_name] = std::visit([](auto&& arg) -> std::type_index { return typeid(arg); }, parameter);
	}

	//
	// Iterate through "real" flags, check if they match. If the map
	// is not empty at the end of this, passed flags are invalid.
	//

	for(const auto& real_arg : to_compare.args) {
		auto it = arg_map.begin();

		if( (it = arg_map.find(real_arg.long_form)) != arg_map.end() || (it = arg_map.find(real_arg.short_form)) != arg_map.end() ) {
			if((it->second.has_value() && it->second.value() == real_arg.type_name) &&
				arg_map.count(real_arg.long_form) + arg_map.count(real_arg.short_form) < 2) {

				arg_map.erase(it);
			}
		}

		else if(real_arg.required){ //if arg is required and not found, fail.
			return false;
		}
	}

	return arg_map.empty();
}

