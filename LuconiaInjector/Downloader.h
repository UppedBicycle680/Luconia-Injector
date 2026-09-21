#pragma once

#define CURL_STATICLIB

#include <curl/curl.h>
#include <windows.h>
#include <iostream>
#include <string>

#include "Config.h"
#include "Utils.h"

#pragma comment(lib, "urlmon.lib")

class Downloader
{
public:
	inline static std::string filePath;
	inline static std::string latestVersion;
public:
	inline static std::string installedVersion;
private:
	static size_t writefunc(void* ptr, size_t size, size_t nmemb, std::string* s)
	{
		s->append(static_cast<char*>(ptr), size * nmemb);
		return size * nmemb;
	}
public:
	static void init()
	{
		filePath = Config::directory + "luconia.dll";
		latestVersion.clear();
		std::ifstream file(Config::directory + "version");
		file >> installedVersion;
		file.close();

		// A manually selected DLL does not depend on the update service.
		if (Config::config["settings"]["use_custom_path"]) return;

		CURLcode result = CURLE_FAILED_INIT;
		CURL* curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, "https://media.luconia.net/version.txt");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &latestVersion);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
			curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
			result = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}

		// Reject failed requests and HTML/error responses, and trim text-file newlines.
		const auto first = latestVersion.find_first_not_of(" \t\r\n");
		const auto last = latestVersion.find_last_not_of(" \t\r\n");
		latestVersion = first == std::string::npos ? "" : latestVersion.substr(first, last - first + 1);
		if (result != CURLE_OK || latestVersion.empty() || latestVersion.size() > 128 ||
			latestVersion.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-") != std::string::npos ||
			latestVersion == "." || latestVersion == "..")
		{
			latestVersion.clear();
			MessageBox(
				NULL,
				L"The Luconia update service could not be reached or returned an invalid version.\n"
				L"The injector will still open. An existing Luconia DLL can be used,\n"
				L"or enable Custom dll and select a compatible client DLL.",
				L"Updates unavailable",
				MB_ICONWARNING | MB_OK
			);
			return;
		}

		download();
		checkUpdate(false);
	}

	static void download()
	{
		if (latestVersion.empty()) return;
		String url = "https://media.luconia.net/" + latestVersion + "/luconia.dll";
		String path = filePath;

		if (!existsFile("version"))
		{
			std::ofstream file(Config::directory + "version");
			file << latestVersion << '\n';
			file.close();
		}

		if (!existsFile("luconia.dll"))
		{
			std::ofstream file(Config::directory + "version");
			file << latestVersion << '\n';
			file.close();

			URLDownloadToFile(NULL, url.to_wstring().c_str(), path.to_wstring().c_str(), NULL, NULL);

			if (!checkFileSize("luconia.dll"))
			{
				int msgBoxID = MessageBox(
					NULL,
					L"Failed to download the dll file\nMake sure that you are connected to the internet\nDo you want to install it again?",
					L"An error occurred",
					MB_ICONERROR | MB_YESNO | MB_DEFBUTTON2
				);

				switch (msgBoxID)
				{
				case IDYES:
					DeleteFileA(filePath.c_str());
					download();
					break;
				case IDNO:
					terminate();
					break;
				}
			}
		}

		std::ifstream file(Config::directory + "version");
		file >> installedVersion;
		file.close();

		if (!checkFileSize("luconia.dll"))
		{
			int msgBoxID = MessageBox(
				NULL,
				L"The dll file size is 0\nMake sure that you are connected to the internet\nDo you want to install it again?",
				L"An error occurred",
				MB_ICONERROR | MB_YESNO | MB_DEFBUTTON2
			);

			switch (msgBoxID)
			{
			case IDYES:
				DeleteFileA(filePath.c_str());
				download();
				break;
			case IDNO:
				terminate();
				break;
			}
		}
	}

	static void checkUpdate(bool askUser)
	{
		// Never delete the installed DLL when the version lookup failed.
		if (latestVersion.empty()) return;
		if (askUser)
		{
			if (installedVersion != latestVersion)
			{
				int msgBoxID = MessageBox(
					NULL,
					L"New Luconia dll version found\nDo you want to update it?\n\nThe dll will be updated on the next start!",
					L"Newer version found",
					MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2
				);

				switch (msgBoxID)
				{
				case IDYES:
					DeleteFileA(filePath.c_str());
					download();
					break;
				}
			}
		}
		else
		{
			if (installedVersion != latestVersion)
			{
				DeleteFileA(filePath.c_str());
				download();
			}
		}
	}

	static bool checkFileSize(std::string name)
	{
		std::ifstream file(Config::directory + name, std::ios::binary);
		file.seekg(0, std::ios::end);
		int size = file.tellg();
		return !(size == 0);
	}

	static bool existsFile(std::string name)
	{
		std::ifstream file(Config::directory + name);
		bool exists = !file ? false : true;
		file.close();
		return exists;
	}
};