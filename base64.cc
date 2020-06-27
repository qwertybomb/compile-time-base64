#include <iostream>

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
	constexpr operator char const *() const { return buf; }
	constexpr operator char *() { return buf; }
	std::size_t constexpr size() const { return N; }
};
template <std::size_t N>
FixedString(char const (&)[N]) -> FixedString<N - 1>;

template <FixedString string>
auto constexpr decode_base64()
{
	std::size_t constexpr string_size = string.size();
	FixedString<(string_size * 3) / 4 - (string_size & 3)> result;
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
			(convert_char(string[i + 1]) << 4) | (convert_char(string[i + 2]) >> 2),
			(convert_char(string[i + 2]) << 6) | (convert_char(string[i + 3])),
		};
		result[j] = bytes[0];
		result[j + 1] = bytes[1];
		if (string[i + 3] != '=')
			result[j + 2] = bytes[2];
	}
	return result;
}

int main()
{
	auto test = decode_base64<"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz\
IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg\
dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu\
dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo\
ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=">();
	std::cout << test << '\n';
}