#include <iostream>
namespace base64
{
	template <std::size_t N>
	struct FixedString
	{
		char buf[N + 1]{};
		constexpr FixedString() = default;
		constexpr FixedString(char const *s)
		{
			for (std::size_t i = 0; i != N; ++i)
				buf[i] = s[i];
		}
		template <std::size_t S>
		constexpr FixedString(FixedString<S> const &other)
		{
			for (std::size_t i = 0; i != other.size(); ++i)
				buf[i] = other.buf[i];
		}
		constexpr operator char const *() const { return buf; }
		constexpr operator char *() { return buf; }
		std::size_t constexpr size() const { return N; }
	};

	template <std::size_t N>
	FixedString(char const (&)[N]) -> FixedString<N - 1>;

	template <FixedString string>
	auto constexpr decode()
	{
		std::size_t constexpr string_size = string.size();
		auto constexpr find_padding = [string_size](){ 
			std::size_t i;
			for(i=0; i < string_size; ++i)
				if(string[i] == '=') break;
			 return i;
		};
		FixedString<(string_size * 3) / 4> result;
		auto constexpr convert_char = [](auto const &ch) {
			if (ch >= 'A' && ch <= 'Z')
				return ch - 65;
			else if (ch >= 'a' && ch <= 'z')
				return ch - 71;
			else if (ch >= '0')
				return ch + 4;
			else
				return ch == '+' ? 62 : 63;
		};
		for (std::size_t i = 0, j = 0; i < string_size; i += 4, j += 3)
		{
			char bytes[3] = {
				(convert_char(string[i]) << 2) | (convert_char(string[i + 1]) >> 4),
				(convert_char(string[i + 1]) << 4) | (convert_char(string[i + 2 >= string_size ? i+1 : i + 2]) >> 2),
				(convert_char(string[i + 2 >= string_size ? i+1 : i+2]) << 6) | (convert_char(string[i + 3 >= string_size ? i+1 :i + 3])),
			};
			result[j] = bytes[0];
			result[j + 1] = bytes[1];
			if (string[i + 3] != '=')
				result[j + 2] = bytes[2];
		}
		return result;
	}

	template <FixedString string>
	auto constexpr encode()
	{
		std::size_t constexpr string_size = string.size();
		std::size_t constexpr result_size_no_padding = (string_size * 4 + 2) / 3;
		std::size_t constexpr result_size = (result_size_no_padding+3)&(-4);
		std::size_t constexpr padding_size = result_size - result_size_no_padding;
		FixedString<(string_size+3)&(-4)> string_with_padding = string;
		FixedString<result_size> result;
		auto constexpr convert_num = [](auto const &num) {
			if (num < 26)
				return static_cast<char>(num + 65);
			else if (num > 25 && num < 52)
				return static_cast<char>(num + 71);
			else if (num > 51)
				return static_cast<char>(num - 4);
			else
				return num == 62 ? '+' : '/';
		};
		for (std::size_t i = 0, j = 0; i < string_size; i += 3, j += 4)
		{
			char bytes[4] = {
				string_with_padding[i] >> 2,
				((string_with_padding[i] & (3)) << 4) | (string_with_padding[i + 1] >> 4),
				((string_with_padding[i + 1] & (15)) << 2) | (string_with_padding[i + 2] >> 6),
				string_with_padding[i + 2] & 63};
			result[j] = convert_num(bytes[0]);
			result[j + 1] = convert_num(bytes[1]);
			result[j + 2] = convert_num(bytes[2]);
			result[j + 3] = convert_num(bytes[3]);
		}
			for (std::size_t i = 0; i < padding_size; ++i)
				result[result_size_no_padding + i] = '=';
		return result;
	}
} // namespace base64

int main()
{

	auto constexpr test = base64::encode<"Man is distinguished, not only by his reason, but by this singular passion from other animals, \
which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable \
generation of knowledge, exceeds the short vehemence of any carnal pleasure...">();

	std::cout << test << '\n';
	std::cout << '\n' << base64::decode<test>() << '\n';
}