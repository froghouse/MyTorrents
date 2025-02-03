/**
 * @brief Simple demonstration program for the TorrentFile parser
 *
 * This program loads and displays metadata from a .torrent file,
 * showing basic usage of the TorrentFile class.
 */

#include <iostream>
#include <torrentfile.hpp>

int main() {
  try {
    // Load and parse the torrent file
    // This example uses Ubuntu 24.04's torrent file
    TorrentFile torrent("ubuntu-24.04.torrent");

    // Display basic torrent metadata
    // These are the core properties that describe the torrent content
    std::cout << "Name: " << torrent.getName() << '\n';
    std::cout << "Announce URL: " << torrent.getAnnounce() << '\n';
    std::cout << "Piece Length: " << torrent.getPieceLength() << " bytes\n";
    std::cout << "Total Size: " << torrent.getTotalSize() << " bytes\n";
    std::cout << "Number of Pieces: " << torrent.getPieces().size() << '\n';

    // Display information about each file in the torrent
    // For single-file torrents, this will show just one entry
    // For multi-file torrents, it shows all files with their paths and sizes
    std::cout << "\nFiles:\n";
    for (const auto &file : torrent.getFiles()) {
      std::cout << file.path << " (" << file.length << " bytes)\n";
    }

  } catch (const std::exception &e) {
    // Handle any errors that occur during parsing
    // This includes file I/O errors and invalid torrent format errors
    std::cerr << "Error: " << e.what() << std::endl;
  }
}
