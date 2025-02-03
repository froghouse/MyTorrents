#include <fstream>
#include <stdexcept>
#include <torrentfile.hpp>

/**
 * @brief Read a .torrent file from disk into a string
 * @param filepath Path to the .torrent file to read
 * @return Raw contents of the file as a string
 * @throws std::runtime_error if the file cannot be opened or read
 *
 * This function reads the entire .torrent file into memory at once.
 * The file is read in binary mode to preserve the exact byte sequence,
 * which is important for Bencode parsing.
 */
std::string readTorrentFile(const std::string &filepath) {
  // Open file in binary mode and position at end for size calculation
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + filepath);
  }

  // Get file size by reading position at end of file
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg); // Reset to beginning of file

  // Allocate buffer of exact file size
  std::string buffer;
  buffer.resize(size);

  // Read entire file into buffer in one operation
  if (!file.read(buffer.data(), size)) {
    throw std::runtime_error("Could not read file: " + filepath);
  }

  return buffer;
}

/**
 * @brief Construct a TorrentFile by parsing a .torrent file
 * @param filepath Path to the .torrent file to parse
 * @throws std::runtime_error if file is invalid or required fields are missing
 */
TorrentFile::TorrentFile(const std::string &filepath) {
  // Read the raw contents of the .torrent file from disk
  std::string torrentData = readTorrentFile(filepath);

  // Parse the Bencode-encoded data into a structured format
  // This will handle all bencode types (integers, strings, lists, and
  // dictionaries)
  BencodeValue result = BencodeParser::parse(torrentData);

  // Validate that the root element is a dictionary
  // According to the BitTorrent specification, all .torrent files must have a
  // dictionary as the root
  if (!result.isDict()) {
    throw std::runtime_error("Invalid torrent file: root must be a dictionary");
  }

  // Get the root dictionary and parse its contents
  // This will extract all metadata including tracker URL, file info, and piece
  // hashes
  const auto &dict = result.getDict();
  parseTorrentDict(dict);
}

// Getter implementations

/**
 * @brief Get the tracker URL that coordinates peers for this torrent
 * @return A reference to the announce URL string used for peer discovery and
 * coordination
 */
const std::string &TorrentFile::getAnnounce() const { return announce; }

/**
 * @brief Get the display name of the torrent content
 * @return A reference to the name string, representing the suggested filename
 * or directory name
 */
const std::string &TorrentFile::getName() const { return name; }

/**
 * @brief Get the size of each piece the file(s) are divided into
 * @return The piece length in bytes, typically a power of 2 (e.g., 16KB, 32KB,
 * 64KB)
 */
int64_t TorrentFile::getPieceLength() const { return pieceLength; }

/**
 * @brief Get the SHA-1 hashes for all pieces in the torrent
 * @return A reference to the vector containing 20-byte SHA-1 hash strings for
 * each piece
 */
const std::vector<std::string> &TorrentFile::getPieces() const {
  return pieces;
}

/**
 * @brief Get information about all files in the torrent
 * @return A reference to the vector of FileInfo structs containing path and
 * size for each file
 */
const std::vector<TorrentFile::FileInfo> &TorrentFile::getFiles() const {
  return files;
}

/**
 * @brief Get the combined size of all files in the torrent
 * @return Total size in bytes of all content in the torrent
 */
int64_t TorrentFile::getTotalSize() const { return totalSize; }

/**
 * @brief Get the client software that created this torrent file
 * @return A reference to the string identifying the creator client (may be
 * empty if not specified)
 */
const std::string &TorrentFile::getCreatedBy() const { return createdBy; }

/**
 * @brief Get when this torrent file was created
 * @return Unix timestamp of torrent creation (0 if not specified)
 */
int64_t TorrentFile::getCreationDate() const { return creationDate; }

/**
 * @brief Check if this torrent contains a single file or multiple files
 * @return true if torrent contains exactly one file, false if it contains
 * multiple files
 */
bool TorrentFile::isSingleFile() const { return singleFile; }

/**
 * @brief Parse the main dictionary of the torrent file containing all metadata
 * @param dict The root Bencode dictionary from the .torrent file
 * @throws std::runtime_error if the required info dictionary is missing or
 * invalid
 */
void TorrentFile::parseTorrentDict(const BencodeValue::Dict &dict) {
  // Parse the tracker's announce URL (required field)
  // This URL is used by clients to report their status and get peer lists
  if (auto it = dict.find("announce");
      it != dict.end() && it->second->isString()) {
    announce = it->second->getString();
  }

  // Parse the creation timestamp (optional field)
  // Stored as a Unix epoch timestamp indicating when the torrent was created
  if (auto it = dict.find("creation date");
      it != dict.end() && it->second->isInt()) {
    creationDate = it->second->getInt();
  }

  // Parse the client that created the torrent (optional field)
  // Usually contains the name and version of the BitTorrent client
  if (auto it = dict.find("created by");
      it != dict.end() && it->second->isString()) {
    createdBy = it->second->getString();
  }

  // Find and validate the info dictionary (required field)
  // The info dictionary contains the core data about files, pieces, and paths
  auto infoIt = dict.find("info");
  if (infoIt == dict.end() || !infoIt->second->isDict()) {
    throw std::runtime_error(
        "Invalid torrent file: missing or invalid info dictionary");
  }

  // Parse the contents of the info dictionary
  // This contains all the file-specific metadata needed for downloading
  parseInfoDict(infoIt->second->getDict());
}

/**
 * @brief Parse the info dictionary containing core torrent metadata
 * @param infoDict The info dictionary extracted from the torrent file
 * @throws std::runtime_error if required fields are missing or invalid
 */
void TorrentFile::parseInfoDict(const BencodeValue::Dict &infoDict) {
  // Parse the size of each piece (required field)
  // Pieces are fixed-size blocks of data that make up the file(s)
  // Typical values are powers of 2 (16KB to 1MB)
  if (auto it = infoDict.find("piece length");
      it != infoDict.end() && it->second->isInt()) {
    pieceLength = it->second->getInt();
  } else {
    throw std::runtime_error("Invalid torrent file: missing piece length");
  }

  // Parse the concatenated SHA-1 hashes of all pieces (required field)
  // Each hash is exactly 20 bytes long and verifies the integrity of a piece
  if (auto it = infoDict.find("pieces");
      it != infoDict.end() && it->second->isString()) {
    std::string piecesStr = it->second->getString();
    // Split the string into individual 20-byte hashes
    // These hashes are used to verify downloaded pieces
    for (size_t i = 0; i < piecesStr.length(); i += 20) {
      pieces.push_back(piecesStr.substr(i, 20));
    }
  } else {
    throw std::runtime_error("Invalid torrent file: missing pieces");
  }

  // Parse the suggested name for the file/directory (required field)
  // This is the default name shown to users in torrent clients
  if (auto it = infoDict.find("name");
      it != infoDict.end() && it->second->isString()) {
    name = it->second->getString();
  }

  // Determine if this is a single-file or multi-file torrent
  // Single-file torrents have a 'length' field
  // Multi-file torrents have a 'files' list instead
  if (auto lengthIt = infoDict.find("length");
      lengthIt != infoDict.end() && lengthIt->second->isInt()) {
    // Single file mode: one file with a specified length
    singleFile = true;
    totalSize = lengthIt->second->getInt();
    files.push_back({name, totalSize}); // Create single FileInfo entry
  } else if (auto filesIt = infoDict.find("files");
             filesIt != infoDict.end() && filesIt->second->isList()) {
    // Multiple files mode: list of files with paths and lengths
    singleFile = false;
    parseFilesList(filesIt->second->getList());
  } else {
    throw std::runtime_error("Invalid torrent file: missing length or files");
  }
}

/**
 * @brief Parse the list of files in a multi-file torrent
 * @param filesList List of dictionaries containing file paths and sizes
 *
 * This method processes each file entry, building the complete path and
 * calculating total torrent size. Invalid entries are skipped silently.
 */
void TorrentFile::parseFilesList(const BencodeValue::List &filesList) {
  // Iterate through each file entry in the files list
  for (const auto &filePtr : filesList) {
    // Each file must be represented by a dictionary
    // Skip invalid entries instead of failing
    if (!filePtr->isDict())
      continue;

    const auto &fileDict = filePtr->getDict();

    // Extract the file length in bytes (required field)
    // Skip entry if length is missing or invalid
    int64_t length = 0;
    if (auto it = fileDict.find("length");
        it != fileDict.end() && it->second->isInt()) {
      length = it->second->getInt();
    } else {
      continue;
    }

    // Build the complete file path from path components
    // The path is stored as a list of strings that should be joined with '/'
    // Example: ["dir1", "dir2", "filename.txt"] becomes
    // "dir1/dir2/filename.txt"
    std::string path;
    if (auto it = fileDict.find("path");
        it != fileDict.end() && it->second->isList()) {
      const auto &pathList = it->second->getList();
      for (size_t i = 0; i < pathList.size(); ++i) {
        // Skip invalid path components
        if (!pathList[i]->isString())
          continue;
        // Add directory separator except for first component
        if (i > 0)
          path += "/";
        path += pathList[i]->getString();
      }
    } else {
      continue;
    }

    // Add valid file entry to our list and update total size
    files.push_back({path, length});
    totalSize += length;
  }
}
