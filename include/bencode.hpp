#ifndef BENCODE_HPP
#define BENCODE_HPP

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

/**
 * @brief A class representing a Bencode value
 *
 * Bencode is a simple encoding format used primarily in the BitTorrent
 * protocol. It supports four data types:
 * - Integers (e.g., i42e)
 * - Strings (e.g., 4:spam)
 * - Lists (e.g., l4:spami42ee)
 * - Dictionaries (e.g., d3:foo3:bare)
 */
class BencodeValue {
public:
  // Define the possible types a bencode value can hold using modern C++ types
  using BencodePtr =
      std::unique_ptr<BencodeValue>;    // Smart pointer for memory management
  using List = std::vector<BencodePtr>; // Ordered list of Bencode values
  using Dict = std::map<std::string, BencodePtr>; // String-keyed dictionary
  using ValueType = std::variant<int64_t, std::string, List,
                                 Dict>; // Type-safe union of possible values

  /**
   * @brief Construct a Bencode integer value
   * @param i The integer value to store
   */
  BencodeValue(int64_t i);

  /**
   * @brief Construct a Bencode string value
   * @param s The string value to store
   */
  BencodeValue(const std::string &s);

  /**
   * @brief Construct a Bencode list value
   * @param l The list to store (moved for efficiency)
   */
  BencodeValue(List &&l);

  /**
   * @brief Construct a Bencode dictionary value
   * @param d The dictionary to store (moved for efficiency)
   */
  BencodeValue(Dict &&d);

  /**
   * @brief Type checking methods to safely determine the stored type
   * @return true if the value is of the queried type
   */
  bool isInt() const;    // Check if value is an integer
  bool isString() const; // Check if value is a string
  bool isList() const;   // Check if value is a list
  bool isDict() const;   // Check if value is a dictionary

  /**
   * @brief Value getters that return the stored value if it matches the
   * requested type
   * @throws std::bad_variant_access if the requested type doesn't match the
   * stored type
   * @return The stored value of the requested type
   */
  int64_t getInt() const;               // Get stored integer
  const std::string &getString() const; // Get stored string
  const List &getList() const;          // Get stored list
  const Dict &getDict() const;          // Get stored dictionary

private:
  ValueType value; // The actual stored value using std::variant
};

/**
 * @brief Parser class for decoding Bencode format
 *
 * This class provides static methods to parse Bencode-encoded data.
 * The parser processes input as a string_view for efficient string handling.
 */
class BencodeParser {
public:
  /**
   * @brief Parse a complete Bencode-encoded string
   * @param input The Bencode-encoded data to parse
   * @return The parsed BencodeValue
   * @throws std::runtime_error if parsing fails
   */
  static BencodeValue parse(std::string_view input);

private:
  /**
   * @brief Helper methods for parsing specific Bencode types
   * @param input The complete input string
   * @param pos Current parsing position (updated as parsing progresses)
   * @return The parsed value of the specific type
   * @throws std::runtime_error if parsing fails
   */
  static BencodeValue parseValue(std::string_view input,
                                 size_t &pos); // Parse any value
  static int64_t parseInt(std::string_view input,
                          size_t &pos); // Parse integer (i42e)
  static std::string parseString(std::string_view input,
                                 size_t &pos); // Parse string (4:spam)
  static BencodeValue::List parseList(std::string_view input,
                                      size_t &pos); // Parse list (l...e)
  static BencodeValue::Dict parseDict(std::string_view input,
                                      size_t &pos); // Parse dict (d...e)
};

#endif // BENCODE_HPP
