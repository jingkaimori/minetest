/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <string>
#include <vector>
#include <list>

#include "irrlichttypes.h"
#include "util/enriched_string.h"
#include "settings.h"

// Chat console related classes

struct ChatLine
{
	// age in seconds
	float age = 0.0f;
	// name of sending player, or empty if sent by server
	EnrichedString name;
	// message text
	EnrichedString text;

	ChatLine(const std::wstring &a_name, const std::wstring &a_text):
		name(a_name),
		text(a_text)
	{
	}

	ChatLine(const EnrichedString &a_name, const EnrichedString &a_text):
		name(a_name),
		text(a_text)
	{
	}
};

struct ChatFormattedFragment
{
	// text string
	EnrichedString text;
	// starting column
	uint32_t column;
	// formatting
	//uint8_t bold:1;
};

struct ChatFormattedLine
{
	// Array of text fragments
	std::vector<ChatFormattedFragment> fragments;
	// true if first line of one formatted ChatLine
	bool first;
};

class ChatBuffer
{
public:
	ChatBuffer(uint32_t scrollback);
	~ChatBuffer() = default;

	// Append chat line
	// Removes oldest chat line if scrollback size is reached
	void addLine(const std::wstring &name, const std::wstring &text);

	// Remove all chat lines
	void clear();

	// Get number of lines currently in buffer.
	uint32_t getLineCount() const;
	// Get reference to i-th chat line.
	const ChatLine& getLine(uint32_t index) const;

	// Increase each chat line's age by dtime.
	void step(float dtime);
	// Delete oldest N chat lines.
	void deleteOldest(uint32_t count);
	// Delete lines older than maxAge.
	void deleteByAge(float maxAge);

	// Get number of rows, 0 if reformat has not been called yet.
	uint32_t getRows() const;
	// Update console size and reformat all formatted lines.
	void reformat(uint32_t cols, uint32_t rows);
	// Get formatted line for a given row (0 is top of screen).
	// Only valid after reformat has been called at least once
	const ChatFormattedLine& getFormattedLine(uint32_t row) const;
	// Scrolling in formatted buffer (relative)
	// positive rows == scroll up, negative rows == scroll down
	void scroll(int32_t rows);
	// Scrolling in formatted buffer (absolute)
	void scrollAbsolute(int32_t scroll);
	// Scroll to bottom of buffer (newest)
	void scrollBottom();
	// Scroll to top of buffer (oldest)
	void scrollTop();

	// Format a chat line for the given number of columns.
	// Appends the formatted lines to the destination array and
	// returns the number of formatted lines.
	uint32_t formatChatLine(const ChatLine& line, uint32_t cols,
			std::vector<ChatFormattedLine>& destination) const;

	void resize(uint32_t scrollback);
protected:
	int32_t getTopScrollPos() const;
	int32_t getBottomScrollPos() const;

private:
	// Scrollback size
	uint32_t m_scrollback;
	// Array of unformatted chat lines
	std::vector<ChatLine> m_unformatted;

	// Number of character columns in console
	uint32_t m_cols = 0;
	// Number of character rows in console
	uint32_t m_rows = 0;
	// Scroll position (console's top line index into m_formatted)
	int32_t m_scroll = 0;
	// Array of formatted lines
	std::vector<ChatFormattedLine> m_formatted;
	// Empty formatted line, for error returns
	ChatFormattedLine m_empty_formatted_line;
};

class ChatPrompt
{
public:
	ChatPrompt(const std::wstring &prompt, uint32_t history_limit);
	~ChatPrompt() = default;

	// Input character or string
	void input(wchar_t ch);
	void input(const std::wstring &str);

	// Add a string to the history
	void addToHistory(const std::wstring &line);

	// Get current line
	std::wstring getLine() const { return m_line; }

	// Get section of line that is currently selected
	std::wstring getSelection() const { return m_line.substr(m_cursor, m_cursor_len); }

	// Clear the current line
	void clear();

	// Replace the current line with the given text
	std::wstring replace(const std::wstring &line);

	// Select previous command from history
	void historyPrev();
	// Select next command from history
	void historyNext();

	// Nick completion
	void nickCompletion(const std::list<std::string>& names, bool backwards);

	// Update console size and reformat the visible portion of the prompt
	void reformat(uint32_t cols);
	// Get visible portion of the prompt.
	std::wstring getVisiblePortion() const;
	// Get cursor position (relative to visible portion). -1 if invalid
	int32_t getVisibleCursorPosition() const;
	// Get length of cursor selection
	int32_t getCursorLength() const { return m_cursor_len; }

	// Cursor operations
	enum CursorOp {
		CURSOROP_MOVE,
		CURSOROP_SELECT,
		CURSOROP_DELETE
	};

	// Cursor operation direction
	enum CursorOpDir {
		CURSOROP_DIR_LEFT,
		CURSOROP_DIR_RIGHT
	};

	// Cursor operation scope
	enum CursorOpScope {
		CURSOROP_SCOPE_CHARACTER,
		CURSOROP_SCOPE_WORD,
		CURSOROP_SCOPE_LINE,
		CURSOROP_SCOPE_SELECTION
	};

	// Cursor operation
	// op specifies whether it's a move or delete operation
	// dir specifies whether the operation goes left or right
	// scope specifies how far the operation will reach (char/word/line)
	// Examples:
	//   cursorOperation(CURSOROP_MOVE, CURSOROP_DIR_RIGHT, CURSOROP_SCOPE_LINE)
	//     moves the cursor to the end of the line.
	//   cursorOperation(CURSOROP_DELETE, CURSOROP_DIR_LEFT, CURSOROP_SCOPE_WORD)
	//     deletes the word to the left of the cursor.
	void cursorOperation(CursorOp op, CursorOpDir dir, CursorOpScope scope);

protected:
	// set m_view to ensure that 0 <= m_view <= m_cursor < m_view + m_cols
	// if line can be fully shown, set m_view to zero
	// else, also ensure m_view <= m_line.size() + 1 - m_cols
	void clampView();

private:
	// Prompt prefix
	std::wstring m_prompt = L"";
	// Currently edited line
	std::wstring m_line = L"";
	// History buffer
	std::vector<std::wstring> m_history;
	// History index (0 <= m_history_index <= m_history.size())
	uint32_t m_history_index = 0;
	// Maximum number of history entries
	uint32_t m_history_limit;

	// Number of columns excluding columns reserved for the prompt
	int32_t m_cols = 0;
	// Start of visible portion (index into m_line)
	int32_t m_view = 0;
	// Cursor (index into m_line)
	int32_t m_cursor = 0;
	// Cursor length (length of selected portion of line)
	int32_t m_cursor_len = 0;

	// Last nick completion start (index into m_line)
	int32_t m_nick_completion_start = 0;
	// Last nick completion start (index into m_line)
	int32_t m_nick_completion_end = 0;
};

class ChatBackend
{
public:
	ChatBackend();
	~ChatBackend() = default;

	// Add chat message
	void addMessage(const std::wstring &name, std::wstring text);
	// Parse and add unparsed chat message
	void addUnparsedMessage(std::wstring line);

	// Get the console buffer
	ChatBuffer& getConsoleBuffer();
	// Get the recent messages buffer
	ChatBuffer& getRecentBuffer();
	// Concatenate all recent messages
	EnrichedString getRecentChat() const;
	// Get the console prompt
	ChatPrompt& getPrompt();

	// Reformat all buffers
	void reformat(uint32_t cols, uint32_t rows);

	// Clear all recent messages
	void clearRecentChat();

	// Age recent messages
	void step(float dtime);

	// Scrolling
	void scroll(int32_t rows);
	void scrollPageDown();
	void scrollPageUp();

	// Resize recent buffer based on settings
	void applySettings();

private:
	ChatBuffer m_console_buffer;
	ChatBuffer m_recent_buffer;
	ChatPrompt m_prompt;
};
