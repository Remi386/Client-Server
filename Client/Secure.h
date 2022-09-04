#pragma once
#include <string>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <algorithm>
#include <sstream>

namespace secure {

	using ustring = std::basic_string<unsigned char>;

	/// <summary>
	/// Converts binary data to hex formated string
	/// </summary>
	/// <param name="buffer"></param>
	/// <param name="size"></param>
	/// <returns></returns>
	std::string bin_to_hex(const unsigned char* buffer, int size)
	{
		std::stringstream ss;
		ss << std::hex;

		for (int i = 0; i < size; ++i) {
			//4 higher bits
			int value = (buffer[i] & 0xF0) >> 4;
			ss << value;

			//4 lower bits
			value = buffer[i] & 0x0F;
			ss << value;
		}
		return ss.str();
	}

	/// <summary>
	/// Calculates SHA256 for given password
	/// </summary>
	/// <param name="rowPassword"></param>
	/// <returns></returns>
	std::string hashPassword(const std::string& rowPassword) 
	{
		ustring convertedPassword;
		convertedPassword.resize(SHA256_DIGEST_LENGTH);
		
		//Call SHA256 algorithm with no salt bytes, 1030 iterations
		int status = PKCS5_PBKDF2_HMAC(rowPassword.c_str(), rowPassword.size(), 
									   nullptr, 0, 1030, EVP_sha256(), 
									   SHA256_DIGEST_LENGTH, convertedPassword.data());

		return status ? bin_to_hex(convertedPassword.c_str(), SHA256_DIGEST_LENGTH) 
					  : std::string();
	}
} // secure