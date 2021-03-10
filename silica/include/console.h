#pragma once

/*
	From github gist:
	https://gist.github.com/ikram-s/990c5e38a3453f6d03e9ace39e0a7c88
*/

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include <numeric>

namespace Silica {

	// Effect structure, to store all ANSII CSI sequences
	// SGR sequences can be merged into one vector of numbers. The numbers are unsigned char s to represent 8bit color channels
	struct Effect {
		std::vector<unsigned char> SGR_params;
		std::vector<std::string> params = {};
		std::string string() {
			std::string buf;
			size_t paramsSize = std::accumulate(params.begin(), params.end(), size_t(0),
				[](size_t x, std::string y) {
					return x + y.size();
				}
			);
			buf.reserve(3 + SGR_params.size() * 4 + paramsSize);
			if (SGR_params.size() > 0) {
				buf += "\033[";
				for (auto i : SGR_params) {
					buf += std::to_string(i);
					buf += ';';
				}
				buf += SGR_params.back();
				buf += 'm';
			}
			for (std::string param : params) {
				buf += "\033[";
				buf += param;
			}
			return buf;
		}
	};

	// You can combine effects together	
	// The effects on the right overrite the ones on the left, due to order
	inline Effect operator+(Effect lhs, Effect rhs) {
		lhs.params.insert(lhs.params.end(), rhs.params.begin(), rhs.params.end());
		lhs.SGR_params.insert(lhs.SGR_params.end(), rhs.SGR_params.begin(), rhs.SGR_params.end());
		return lhs;
	}

	// List of all ANSII Select Graphic Rendition effects supported.
  	// Changes foreground color of text
	inline Effect foreground(unsigned char r, unsigned char g, unsigned char b) {
		return {{38,2,r,g,b}};
	}
  
	// Changes background color of text
	inline Effect background(unsigned char r, unsigned char g, unsigned char b) {
		return {{48,2,r,g,b}};
	}

	inline Effect clear       = {{0}};
	inline Effect bold        = {{1}};
	inline Effect light       = {{2}};
	inline Effect italic      = {{3}};
	inline Effect underline   = {{4}};
	inline Effect slow_blink  = {{5}};
	inline Effect fast_blink  = {{6}};
	
	// older, predefined terminal colors, (16 only) 
	
	// 3/4 bit dark foreground colors
	namespace darkfg {
		inline Effect black   = {{30}};
		inline Effect red     = {{31}};
		inline Effect green   = {{32}};
		inline Effect yellow  = {{33}};
		inline Effect blue    = {{34}};
		inline Effect magenta = {{35}};
		inline Effect cyan    = {{36}};
		inline Effect white   = {{37}};
	};

	// 3/4 bit dark background colors
	namespace darkbg {
		inline Effect black   = {{40}};
		inline Effect red     = {{41}};
		inline Effect green   = {{42}};
		inline Effect yellow  = {{43}};
		inline Effect blue    = {{44}};
		inline Effect magenta = {{45}};
		inline Effect cyan    = {{46}};
		inline Effect white   = {{47}};
	};

	// 3/4 bit light/bright foreground colors
	namespace lightfg {	
		inline Effect black   = {{80}};
		inline Effect red     = {{81}};
		inline Effect green   = {{82}};
		inline Effect yellow  = {{83}};
		inline Effect blue    = {{84}};
		inline Effect magenta = {{85}};
		inline Effect cyan    = {{86}};
		inline Effect white   = {{87}};
	};

	// 3/4 bit light/bright foreground colors
	namespace lightbg {
		inline Effect black   = {{100}};
		inline Effect red     = {{101}};
		inline Effect green   = {{102}};
		inline Effect yellow  = {{103}};
		inline Effect blue    = {{104}};
		inline Effect magenta = {{105}};
		inline Effect cyan    = {{106}};
		inline Effect white   = {{107}};
	};

	// Effects to control cursor position
	namespace pos {
		inline Effect up       (int n = 1) { return { {},{std::to_string(n) + "A"} }; }; // move cursor up by n
		inline Effect down     (int n = 1) { return { {},{std::to_string(n) + "B"} }; }; // move cursor down by n
		inline Effect forward  (int n = 1) { return { {},{std::to_string(n) + "C"} }; }; // move cursor forward by n
		inline Effect back     (int n = 1) { return { {},{std::to_string(n) + "D"} }; }; // move cursor back by n
		inline Effect nextline (int n = 1) { return { {},{std::to_string(n) + "E"} }; }; // move cursor to the beginning of the line n lines up
		inline Effect prevline (int n = 1) { return { {},{std::to_string(n) + "F"} }; }; // move cursor to the beginning of the line n lines down
		inline Effect hor_abs  (int n = 1) { return { {},{std::to_string(n) + "G"} }; }; // move cursor to column
	}
}