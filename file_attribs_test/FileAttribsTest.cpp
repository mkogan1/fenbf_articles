/*
g++ --version
g++ (GCC) 13.2.1 20231205 (Red Hat 13.2.1-6)

g++ -std=c++20 -o fileattribstest FileAttribsTest.cpp

mkdir -p /tmp/bench
time (for i in {1..200000}; do touch /tmp/bench/file$i.txt; done)
  ( for i in {1..200000}; do; touch /tmp/bench/file$i.txt; done; )  32.25s user 213.19s system 100% cpu 4:04.26 total

sync ; sudo sysctl vm.drop_caches=3 ; sudo sysctl vm.compact_memory=1
./fileattribstest /tmp/bench
  Method                         Time (seconds)       Speedup Factor      
  std::filesystem                0.7416968160         1.000 
*/


// #include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <string.h>
#include <algorithm>
#include <format>

struct FileInfo {
        std::string fileName;
        std::filesystem::file_time_type lastWriteTime;
};
/*
void BenchmarkFindFirstFileEx(const std::string& directory, std::vector<FileInfo>& files, FINDEX_INFO_LEVELS infoLevel) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFileEx((directory + "\\*").c_str(), infoLevel, &findFileData, FindExSearchNameMatch, NULL, 0);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::cerr << "FindFirstFileEx failed (" << GetLastError() << ")\n";
		return;
	}

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			FileInfo fileInfo;
			fileInfo.fileName = findFileData.cFileName;
			fileInfo.lastWriteTime = findFileData.ftLastWriteTime;
			files.push_back(fileInfo);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}
*/
/*
void BenchmarkGetFileAttributesEx(const std::string& directory, std::vector<FileInfo>& files) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((directory + "\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::cerr << "FindFirstFile failed (" << GetLastError() << ")\n";
		return;
	}

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
			if (GetFileAttributesEx((directory + "\\" + findFileData.cFileName).c_str(), GetFileExInfoStandard, &fileAttributeData)) {
				FileInfo fileInfo;
				fileInfo.fileName = findFileData.cFileName;
				fileInfo.lastWriteTime = fileAttributeData.ftLastWriteTime;
				files.push_back(fileInfo);
			}
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}
*/
void BenchmarkStdFilesystem(const std::string& directory, std::vector<FileInfo>& files, size_t &count) {
        std::cout << "BenchmarkStdFilesystem: ";
        count=0;
	for (const auto& entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file()) {
                        // std::cout << ".";
			FileInfo fileInfo;
			fileInfo.fileName = entry.path().filename().string();
			// https://developercommunity.visualstudio.com/t/stdfilesystemfile-time-type-does-not-allow-easy-co/251213
			auto ftime = std::filesystem::last_write_time(entry);
			// memcpy(&fileInfo.lastWriteTime, &ftime, sizeof(std::filesystem::file_time_type));
			// files.push_back(fileInfo);
		}
                count++;
	}
        std::cout << "\n";
}

void PrintResultsTable(const std::vector<std::pair<std::string, double>>& results, size_t count) {
	// Determine the best result
	auto best_result = std::min_element(results.begin(), results.end(), [](const auto& a, const auto& b) {
		return a.second > b.second;
		});
	double best_time = best_result->second;

	// Print the table header
	std::cout << std::format("{:<30} {:<20} {:<20} {:<20}\n", "Method", "Time (seconds)", "IOPS", "Speedup Factor");

	// Print the results
	for (const auto& result : results) {
		std::cout << std::format("{:<30} {:<20.10f} {:<20.2f} {:<20.3f}\n", result.first, result.second, count/result.second, (best_time / result.second));
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
		return 1;
	}

	std::string directory = argv[1];

	std::vector<FileInfo> filesFindFirstFileExBasic;
	filesFindFirstFileExBasic.reserve(1000);
	std::vector<FileInfo> filesFindFirstFileExStandard;
	filesFindFirstFileExStandard.reserve(1000);
	std::vector<FileInfo> filesGetFileAttributesEx;
	filesGetFileAttributesEx.reserve(1000);
	std::vector<FileInfo> filesStdFilesystem;
	filesStdFilesystem.reserve(1000000);

	// Benchmark FindFirstFileEx (Basic)
	// auto start = std::chrono::high_resolution_clock::now();
	// BenchmarkFindFirstFileEx(directory, filesFindFirstFileExBasic, FindExInfoBasic);
	// auto end = std::chrono::high_resolution_clock::now();
	// std::chrono::duration<double> elapsedFindFirstFileExBasic = end - start;

	// Benchmark FindFirstFileEx (Standard)
	// start = std::chrono::high_resolution_clock::now();
	// BenchmarkFindFirstFileEx(directory, filesFindFirstFileExStandard, FindExInfoStandard);
	// end = std::chrono::high_resolution_clock::now();
	// std::chrono::duration<double> elapsedFindFirstFileExStandard = end - start;

	// Benchmark GetFileAttributesEx
	// start = std::chrono::high_resolution_clock::now();
	// BenchmarkGetFileAttributesEx(directory, filesGetFileAttributesEx);
	// end = std::chrono::high_resolution_clock::now();
	// std::chrono::duration<double> elapsedGetFileAttributesEx = end - start;

	// Benchmark std::filesystem
	auto start = std::chrono::high_resolution_clock::now();
        size_t count;
	BenchmarkStdFilesystem(directory, filesStdFilesystem, count);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedStdFilesystem = end - start;

	// Collect results
	std::vector<std::pair<std::string, double>> results = {
		// {"FindFirstFileEx (Basic)", elapsedFindFirstFileExBasic.count()},
		// {"FindFirstFileEx (Standard)", elapsedFindFirstFileExStandard.count()},
		// {"GetFileAttributesEx", elapsedGetFileAttributesEx.count()},
		{"std::filesystem", elapsedStdFilesystem.count()}
	};

	// Print results in a table format
        std::cout << "IO ops count = " << count << "\n";
        PrintResultsTable(results, count);
}
