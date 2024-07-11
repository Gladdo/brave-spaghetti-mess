#include <vector>
#include <string>

std::vector<unsigned char> load_image_to_unsigned_char_vector(std::string filename, int* img_width, int* img_height);
std::string load_multiline_txt_to_string(std::string filename);
std::vector<float> load_txt_to_float_vector(std::string filename);