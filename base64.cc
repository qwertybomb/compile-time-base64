#include <iostream>
#include <cstring>
namespace base64
{
template <std::size_t N>
struct FixedString
{
	char buf[N + 1] {};
	constexpr FixedString() = default;
	constexpr FixedString(char const *s)
	{
		std::copy(s, s + N, buf);
	}

	template <std::size_t S>
	constexpr FixedString(FixedString<S> const &other)
	{
		std::copy(other.buf, other.buf + std::min(S, N), buf);
	}

	auto constexpr operator == (FixedString const &other) const
	{
		return std::equal(buf, buf + N, other.buf);
	}

	auto constexpr operator[](std::size_t index) const { return buf[index]; }
	auto constexpr &operator[](std::size_t index) { return buf[index]; }
	std::size_t constexpr size() const { return N; }

	friend auto &operator<<(std::ostream& stream, const FixedString& string)
	{
		stream << string.buf;
		return stream;
	}
};

template <std::size_t N>
FixedString(char const (&)[N]) -> FixedString < N - 1 >;

namespace detail
{
auto constexpr convert_char (char const ch) 
{
		if (ch >= 'A' && ch <= 'Z')
			return ch - 65;
		else if (ch >= 'a' && ch <= 'z')
			return ch - 71;
		else if (ch >= '0')
			return ch + 4;
		else
			return ch == '+' ? 62 : 63;
};

auto constexpr convert_number (char const num)
{
		if (num < 26)
			return static_cast<char>(num + 65);
		else if (num > 25 && num < 52)
			return static_cast<char>(num + 71);
		else if (num > 51)
			return static_cast<char>(num - 4);
		else
			return num == 62 ? '+' : '/';
};
}

template <FixedString string>
auto constexpr decode()
{
	std::size_t constexpr string_size = string.size();
	auto constexpr find_padding = [string_size]() {
		return std::distance(string.buf, 
               std::find(string.buf, string.buf + string_size, '='));
	};
	FixedString <find_padding() * 3 / 4 > result;
	for (std::size_t i = 0, j = 0; i < string_size; i += 4, j += 3)
	{
		char bytes[3] = {
			static_cast<char>(detail::convert_char(string[i]) << 2
			| detail::convert_char(string[i + 1]) >> 4),
			static_cast<char>(detail::convert_char(string[i + 1]) << 4
			| detail::convert_char(string[i + 2]) >> 2),
			static_cast<char>(detail::convert_char(string[i + 2]) << 6
			| detail::convert_char(string[i + 3])),
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
	std::size_t constexpr result_size = (result_size_no_padding + 3) & (-4);
	std::size_t constexpr padding_size = result_size - result_size_no_padding;
	FixedString <(string_size + 2) / 3 * 3> constexpr string_with_padding = string;
	FixedString<result_size> result;
	for (std::size_t i = 0, j = 0; i < string_size; i += 3, j += 4)
	{
		/* convert every 3 bytes to 4 6 bit numbers
		 * 8 * 3 = 24
		 * 6 * 4 = 24
		 */
		char bytes[4] = {
			static_cast<char>(string_with_padding[i] >> 2),
			static_cast<char>((string_with_padding[i]
			& 3) << 4
			| string_with_padding[i + 1] >> 4),
			static_cast<char>((string_with_padding[i + 1] & 15) << 2
			| string_with_padding[i + 2] >> 6),
			static_cast<char>(string_with_padding[i + 2] & 63)
		};
		std::transform(bytes, bytes + 4, result.buf + j, detail::convert_number);
	}
	std::fill_n(result.buf + result_size_no_padding, padding_size, '=');

	return result;
}
} // namespace base64

/* here is a test */
int main()
{

	auto constexpr input_text = base64::FixedString { "Man is distinguished, not only by his reason, but by this singular passion from other animals, \
which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable \
generation of knowledge, exceeds the short vehemence of any carnal pleasure." };
	auto constexpr encoded = base64::encode<input_text>();
	auto constexpr decoded = base64::decode<encoded>();
	static_assert(decoded == input_text);
	std::cout << encoded << '\n';
}