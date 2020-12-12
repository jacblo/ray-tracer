#include <vector>
#include <fstream>

struct image
{   
    image(int width, int height)
    :   w(width), h(height), rgb(w * h * 3)
    {}
    uint8_t & r(int x, int y) { return rgb[(x + y*w)*3 + 2]; }
    uint8_t & g(int x, int y) { return rgb[(x + y*w)*3 + 1]; }
    uint8_t & b(int x, int y) { return rgb[(x + y*w)*3 + 0]; }

    int w, h;
    std::vector<uint8_t> rgb;
};

template<class Stream>
Stream & operator<<(Stream & out, image const& img)
{   
    uint32_t w = img.w, h = img.h;
    uint32_t pad = w * -3 & 3;
    uint32_t total = 54 + 3*w*h + pad*h;
    uint32_t head[13] = {total, 0, 54, 40, w, h, (24<<16)|1};
    char const* rgb = (char const*)img.rgb.data();

    out.write("BM", 2);
    out.write((char*)head, 52);
    for(uint32_t i=0 ; i<h ; i++)
    {   out.write(rgb + (3 * w * i), 3 * w);
        out.write((char*)&pad, pad);
    }
    return out;
}

int main()
{
    image img(100, 100);
    for(int x=0 ; x<100 ; x++)
    {   for(int y=0 ; y<100 ; y++)
        {   img.r(x,y) = x;
            img.g(x,y) = y;
            img.b(x,y) = 100-x;
        }
    }
    std::ofstream("/tmp/out.bmp") << img;
}