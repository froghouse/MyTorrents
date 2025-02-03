#ifndef TORRENTFILE_HPP
#define TORRENTFILE_HPP

#include <bencode.hpp>
#include <string>
#include <vector>

/**
 * @brief Read and validate a .torrent file from disk
 * @param filepath Path to the .torrent file
 * @return Raw content of the torrent file as a string
 * @throws std::runtime_error if file cannot be read
 */
std::string readTorrentFile(const std::string &filepath);

/**
 * @brief Class representing a parsed BitTorrent metadata (.torrent) file
 *
 * This class parses and provides access to the metadata contained in a .torrent
 * file, including tracker information, file details, and piece hashes needed
 * for the BitTorrent protocol. The metadata is stored in Bencode format and
 * includes both required and optional fields as specified in the BitTorrent
 * protocol specification.
 */
class TorrentFile {
public:
  /**
   * @brief Structure containing information about a file in the torrent
   *
   * For single-file torrents, only one FileInfo is used.
   * For multi-file torrents, each file in the torrent has its own FileInfo.
   */
  struct FileInfo {
    std::string path; // Full path of the file within the torrent
    int64_t length;   // Size of the file in bytes
  };

  /**
   * @brief Construct a TorrentFile by parsing a .torrent file
   * @param filepath Path to the .torrent file to parse
   * @throws std::runtime_error if file is invalid or required fields are
   * missing
   */
  explicit TorrentFile(const std::string &filepath);

  // Getter methods for torrent metadata

  /**
   * @brief Get the tracker URL (announce URL)
   * @return URL of the tracker that coordinates peers for this torrent
   */
  const std::string &getAnnounce() const;

  /**
   * @brief Get the name of the torrent
   * @return Name of the file/directory that the torrent creates
   */
  const std::string &getName() const;

  /**
   * @brief Get the piece length used in the torrent
   * @return Size of each piece in bytes (typically a power of 2)
   */
  int64_t getPieceLength() const;

  /**
   * @brief Get the list of piece hashes
   * @return Vector of SHA-1 hashes, one for each piece of the torrent
   */
  const std::vector<std::string> &getPieces() const;

  /**
   * @brief Get information about all files in the torrent
   * @return Vector of FileInfo structures describing each file
   */
  const std::vector<FileInfo> &getFiles() const;

  /**
   * @brief Get the total size of all files in the torrent
   * @return Combined size in bytes of all files
   */
  int64_t getTotalSize() const;

  /**
   * @brief Get the client that created the torrent (optional)
   * @return Name/version of the client that created the torrent
   */
  const std::string &getCreatedBy() const;

  /**
   * @brief Get the creation timestamp of the torrent (optional)
   * @return Unix timestamp when the torrent was created
   */
  int64_t getCreationDate() const;

  /**
   * @brief Check if this is a single-file or multi-file torrent
   * @return true if torrent contains exactly one file, false if it contains
   * multiple files
   */
  bool isSingleFile() const;

private:
  // Torrent metadata fields
  std::string announce;            // Tracker URL
  std::string name;                // Name of torrent (file/directory name)
  int64_t pieceLength = 0;         // Size of each piece in bytes
  std::vector<std::string> pieces; // SHA-1 hashes of all pieces
  std::vector<FileInfo> files;     // Information about each file
  int64_t totalSize = 0;           // Combined size of all files
  std::string createdBy;           // Client that created the torrent
  int64_t creationDate = 0;        // Creation timestamp
  bool singleFile = true; // Whether torrent contains one or multiple files

  /**
   * @brief Parse the main dictionary of the torrent file
   * @param dict The Bencode dictionary containing all torrent metadata
   * @throws std::runtime_error if required fields are missing or invalid
   */
  void parseTorrentDict(const BencodeValue::Dict &dict);

  /**
   * @brief Parse the 'info' dictionary containing file information
   * @param infoDict The Bencode dictionary containing file metadata
   * @throws std::runtime_error if required fields are missing or invalid
   */
  void parseInfoDict(const BencodeValue::Dict &infoDict);

  /**
   * @brief Parse the list of files in a multi-file torrent
   * @param filesList The Bencode list containing file information
   * @throws std::runtime_error if file information is invalid
   */
  void parseFilesList(const BencodeValue::List &filesList);
};

#endif // TORRENTFILE_HPP
