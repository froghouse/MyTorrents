#include <bencode.hpp>
#include <cctype>
#include <stdexcept>

/**
 * @brief Initialize a BencodeValue with an integer
 * @param i The integer value to store
 *
 * Example Bencode format: i42e
 */
BencodeValue::BencodeValue(int64_t i) : value(i) {}

/**
 * @brief Initialize a BencodeValue with a string
 * @param s The string value to store
 *
 * Example Bencode format: 4:spam
 * Note: The actual Bencode encoding will include the string length prefix
 */
BencodeValue::BencodeValue(const std::string &s) : value(s) {}

/**
 * @brief Initialize a BencodeValue with a list
 * @param l The list to store (moved to avoid copying)
 *
 * Example Bencode format: l4:spami42ee
 * Uses move semantics for efficient transfer of the list ownership
 */
BencodeValue::BencodeValue(List &&l) : value(std::move(l)) {}

/**
 * @brief Initialize a BencodeValue with a dictionary
 * @param d The dictionary to store (moved to avoid copying)
 *
 * Example Bencode format: d3:foo3:bare
 * Uses move semantics for efficient transfer of the dictionary ownership
 */
BencodeValue::BencodeValue(Dict &&d) : value(std::move(d)) {}

/**
 * @brief Check if the stored value is an integer
 * @return true if the value is an integer, false otherwise
 *
 * Uses std::holds_alternative to safely check if the variant holds an int64_t
 */
bool BencodeValue::isInt() const {
  return std::holds_alternative<int64_t>(value);
}

/**
 * @brief Check if the stored value is a string
 * @return true if the value is a string, false otherwise
 *
 * Uses std::holds_alternative to safely check if the variant holds a string
 */
bool BencodeValue::isString() const {
  return std::holds_alternative<std::string>(value);
}

/**
 * @brief Check if the stored value is a list
 * @return true if the value is a list, false otherwise
 *
 * Uses std::holds_alternative to safely check if the variant holds a List
 */
bool BencodeValue::isList() const {
  return std::holds_alternative<List>(value);
}

/**
 * @brief Check if the stored value is a dictionary
 * @return true if the value is a dictionary, false otherwise
 *
 * Uses std::holds_alternative to safely check if the variant holds a Dict
 */
bool BencodeValue::isDict() const {
  return std::holds_alternative<Dict>(value);
}

/**
 * @brief Get the stored integer value
 * @return The stored integer
 * @throws std::bad_variant_access if the stored value is not an integer
 *
 * Uses std::get to extract the int64_t from the variant
 */
int64_t BencodeValue::getInt() const { return std::get<int64_t>(value); }

/**
 * @brief Get the stored string value
 * @return Reference to the stored string
 * @throws std::bad_variant_access if the stored value is not a string
 *
 * Uses std::get to extract the string from the variant
 * Returns const reference to avoid copying
 */
const std::string &BencodeValue::getString() const {
  return std::get<std::string>(value);
}

/**
 * @brief Get the stored list value
 * @return Reference to the stored list
 * @throws std::bad_variant_access if the stored value is not a list
 *
 * Uses std::get to extract the List from the variant
 * Returns const reference to avoid copying
 */
const BencodeValue::List &BencodeValue::getList() const {
  return std::get<List>(value);
}

/**
 * @brief Get the stored dictionary value
 * @return Reference to the stored dictionary
 * @throws std::bad_variant_access if the stored value is not a dictionary
 *
 * Uses std::get to extract the Dict from the variant
 * Returns const reference to avoid copying
 */
const BencodeValue::Dict &BencodeValue::getDict() const {
  return std::get<Dict>(value);
}

/**
 * @brief Parse a complete Bencode-encoded string
 * @param input The Bencode-encoded data to parse
 * @return Parsed BencodeValue containing the root element
 * @throws std::runtime_error if parsing fails
 *
 * This is the main entry point for parsing Bencode data.
 * It initializes parsing from position 0 and delegates to parseValue.
 */
BencodeValue BencodeParser::parse(std::string_view input) {
  size_t pos = 0;
  return parseValue(input, pos);
}

/**
 * @brief Parse a single Bencode value starting at the given position
 * @param input The complete Bencode-encoded input string
 * @param pos Current parsing position (updated as parsing progresses)
 * @return Parsed BencodeValue
 * @throws std::runtime_error if parsing fails or input is invalid
 *
 * This method determines the type of the next value based on the first
 * character:
 * - Digit: String (e.g., "4:spam")
 * - 'i': Integer (e.g., "i42e")
 * - 'l': List (e.g., "l...e")
 * - 'd': Dictionary (e.g., "d...e")
 *
 * The pos parameter is passed by reference and updated as parsing progresses,
 * allowing sequential parsing of compound structures like lists and
 * dictionaries.
 */
BencodeValue BencodeParser::parseValue(std::string_view input, size_t &pos) {
  // Check for unexpected end of input
  if (pos >= input.size()) {
    throw std::runtime_error("Unexpected end of input");
  }

  // Check first character to determine value type
  char c = input[pos];
  if (std::isdigit(c)) {
    return parseString(input, pos); // String: starts with length prefix
  }
  switch (c) {
  case 'i':
    return parseInt(input, pos); // Integer: starts with 'i'
  case 'l':
    return parseList(input, pos); // List: starts with 'l'
  case 'd':
    return parseDict(input, pos); // Dictionary: starts with 'd'
  default:
    throw std::runtime_error("Invalid value type");
  }
}

/**
 * @brief Parse a Bencode-encoded integer value
 * @param input The complete Bencode-encoded input string
 * @param pos Current parsing position (updated as parsing progresses)
 * @return The parsed integer value
 * @throws std::runtime_error if the integer format is invalid
 *
 * Parses integers in Bencode format: iXe where X is the number
 * Examples:
 * - i42e    -> 42
 * - i-42e   -> -42
 * - i0e     -> 0
 *
 * Format rules:
 * - Must start with 'i' and end with 'e'
 * - May be negative (starts with '-')
 * - No leading zeros allowed (except for 0 itself)
 * - Must contain at least one digit
 */
int64_t BencodeParser::parseInt(std::string_view input, size_t &pos) {
  // Verify and skip the leading 'i' character
  if (pos >= input.size() || input[pos++] != 'i') {
    throw std::runtime_error("Invalid integer format");
  }

  // Check for and handle negative sign
  bool negative = false;
  if (pos < input.size() && input[pos] == '-') {
    negative = true;
    pos++;
  }

  // Ensure at least one digit exists
  if (pos >= input.size() || !std::isdigit(input[pos])) {
    throw std::runtime_error("Invalid integer format: no digits");
  }

  // Enforce no leading zeros rule (special case: single '0' is allowed)
  if (input[pos] == '0' && pos + 1 < input.size() && input[pos + 1] != 'e') {
    throw std::runtime_error("Invalid integer format: leading zeros");
  }

  // Parse the digits into an integer value
  int64_t result = 0;
  while (pos < input.size() && std::isdigit(input[pos])) {
    result = result * 10 + (input[pos] - '0');
    pos++;
  }

  // Verify and skip the trailing 'e' character
  if (pos >= input.size() || input[pos++] != 'e') {
    throw std::runtime_error("Invalid integer format: missing 'e'");
  }

  // Apply negative sign if present
  return negative ? -result : result;
}

/**
 * @brief Parse a Bencode-encoded string value
 * @param input The complete Bencode-encoded input string
 * @param pos Current parsing position (updated as parsing progresses)
 * @return The parsed string value
 * @throws std::runtime_error if the string format is invalid
 *
 * Parses strings in Bencode format: <length>:<string>
 * Examples:
 * - 4:spam -> "spam"
 * - 0:     -> "" (empty string)
 * - 5:hello -> "hello"
 *
 * Format rules:
 * - Must start with a length prefix (non-negative integer)
 * - Length must be followed by a colon
 * - No leading zeros in length (except for 0 itself)
 * - Must have exactly the specified number of characters after the colon
 * - String can contain any bytes (including nulls and non-printable chars)
 */
std::string BencodeParser::parseString(std::string_view input, size_t &pos) {
  // Locate the colon separator between length and string content
  size_t colonPos = input.find(':', pos);
  if (colonPos == std::string_view::npos) {
    throw std::runtime_error("Invalid string format: missing colon");
  }

  // Extract and parse the length prefix
  std::string_view lengthStr = input.substr(pos, colonPos - pos);
  size_t length = 0;

  // Enforce no leading zeros rule in length (special case: single '0' is
  // allowed)
  if (lengthStr.length() > 1 && lengthStr[0] == '0') {
    throw std::runtime_error("Invalid string length: leading zeros");
  }

  // Convert length string to integer, validating digits
  for (char c : lengthStr) {
    if (!std::isdigit(c)) {
      throw std::runtime_error("Invalid string length: non-digit character");
    }
    length = length * 10 + (c - '0');
  }

  // Move position past the colon
  pos = colonPos + 1;

  // Verify we have enough remaining characters to satisfy the length
  if (pos + length > input.size()) {
    throw std::runtime_error("Invalid string: insufficient characters");
  }

  // Extract the actual string content based on the parsed length
  std::string result(input.substr(pos, length));
  pos += length; // Advance position past the string content

  return result;
}

/**
 * @brief Parse a Bencode-encoded list value
 * @param input The complete Bencode-encoded input string
 * @param pos Current parsing position (updated as parsing progresses)
 * @return List of parsed BencodeValues
 * @throws std::runtime_error if the list format is invalid
 *
 * Parses lists in Bencode format: l<value1><value2>...e
 * Examples:
 * - le           -> [] (empty list)
 * - l4:spami42ee -> ["spam", 42]
 * - li1ei2ei3ee  -> [1, 2, 3]
 *
 * Format rules:
 * - Must start with 'l' and end with 'e'
 * - Can contain zero or more Bencode values of any type
 * - Values can be mixed types
 */
BencodeValue::List BencodeParser::parseList(std::string_view input,
                                            size_t &pos) {
  // Verify and skip the leading 'l' character
  if (pos >= input.size() || input[pos++] != 'l') {
    throw std::runtime_error("Invalid list format");
  }

  BencodeValue::List result;

  // Parse elements until we reach the end marker 'e'
  while (pos < input.size() && input[pos] != 'e') {
    // Parse each value and store it in a unique_ptr for memory safety
    result.push_back(std::make_unique<BencodeValue>(parseValue(input, pos)));
  }

  // Verify and skip the trailing 'e' character
  if (pos >= input.size() || input[pos++] != 'e') {
    throw std::runtime_error("Invalid list format: missing 'e'");
  }

  return result;
}

/**
 * @brief Parse a Bencode-encoded dictionary value
 * @param input The complete Bencode-encoded input string
 * @param pos Current parsing position (updated as parsing progresses)
 * @return Dictionary of string keys to parsed BencodeValues
 * @throws std::runtime_error if the dictionary format is invalid
 *
 * Parses dictionaries in Bencode format: d<key1><value1><key2><value2>...e
 * Examples:
 * - de                     -> {} (empty dictionary)
 * - d3:foo3:bar3:spami42ee -> {"foo": "bar", "spam": 42}
 *
 * Format rules:
 * - Must start with 'd' and end with 'e'
 * - Keys must be strings
 * - Keys must be unique
 * - Keys appear in sorted order in the Bencode format
 * - Values can be any Bencode type
 */
BencodeValue::Dict BencodeParser::parseDict(std::string_view input,
                                            size_t &pos) {
  // Verify and skip the leading 'd' character
  if (pos >= input.size() || input[pos++] != 'd') {
    throw std::runtime_error("Invalid dictionary format");
  }

  BencodeValue::Dict result;

  // Parse key-value pairs until we reach the end marker 'e'
  while (pos < input.size() && input[pos] != 'e') {
    // Dictionary keys must be strings, so first character must be a digit
    if (!std::isdigit(input[pos])) {
      throw std::runtime_error("Invalid dictionary key: must be string");
    }

    // Parse the key string
    std::string key = parseString(input, pos);

    // Parse the associated value
    auto value = std::make_unique<BencodeValue>(parseValue(input, pos));

    // Attempt to insert the key-value pair, checking for duplicates
    if (!result.insert({std::move(key), std::move(value)}).second) {
      throw std::runtime_error("Duplicate dictionary key");
    }
  }

  // Verify and skip the trailing 'e' character
  if (pos >= input.size() || input[pos++] != 'e') {
    throw std::runtime_error("Invalid dictionary format: missing 'e'");
  }

  return result;
}
