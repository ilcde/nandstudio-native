// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "core.hpp"
#include <algorithm>
namespace nand {
// Additional bitmap authoring utility. Hack pixels are LSB-first within words.
class Bitmap {
    int width_=48,height_=32;
    std::vector<unsigned char> pixels_=std::vector<unsigned char>(48*32);
public:
    int width()const{return width_;} int height()const{return height_;}
    bool pixel(int x,int y)const{return x>=0&&y>=0&&x<width_&&y<height_&&pixels_[y*width_+x];}
    void set(int x,int y,bool value){if(x<0||y<0||x>=width_||y>=height_)return;pixels_[y*width_+x]=value;}
    void resize(int w,int h){
        if(w<1||w>512||h<1||h>256)throw Error("Bitmap dimensions must be 1..512 by 1..256");
        std::vector<unsigned char> next(w*h);
        for(int y=0;y<std::min(h,height_);++y)for(int x=0;x<std::min(w,width_);++x)next[y*w+x]=pixel(x,y);
        pixels_=std::move(next);width_=w;height_=h;
    }
    void clear(){std::fill(pixels_.begin(),pixels_.end(),0);}
    void invert(){for(auto& p:pixels_)p=!p;}
    void flip(){auto old=*this;for(int y=0;y<height_;++y)for(int x=0;x<width_;++x)set(x,y,old.pixel(width_-1-x,y));}
    void shift(int dx,int dy){auto old=*this;for(int y=0;y<height_;++y)for(int x=0;x<width_;++x)set(x,y,old.pixel(x-dx,y-dy));}
    void rotate(){if(width_!=height_)throw Error("Rotation requires a square canvas");auto old=*this;for(int y=0;y<height_;++y)for(int x=0;x<width_;++x)set(x,y,old.pixel(y,width_-1-x));}
    Word word(int column,int row)const{Word value=0;for(int bit=0;bit<16;++bit)if(pixel(column*16+bit,row))value|=Word(1u<<bit);return value;}
    std::string assembly()const{
        std::string out="// NandStudio bitmap: full canvas; absolute screen origin\n";
        for(int y=0;y<height_;++y)for(int c=0;c<(width_+15)/16;++c){const auto v=word(c,y);if(v<32768)out+="@"+std::to_string(v)+"\nD=A\n";else out+="@"+std::to_string(65535-v)+"\nD=A\nD=!D\n";out+="@"+std::to_string(16384+y*32+c)+"\nM=D\n";}
        return out+"(BITMAP_END)\n@BITMAP_END\n0;JMP\n";
    }
    std::string jack()const{
        std::string out="// NandStudio bitmap: full canvas; location is a screen-word offset\nclass Bitmap {\n    function void draw(int location) {\n";
        for(int y=0;y<height_;++y)for(int c=0;c<(width_+15)/16;++c){auto v=word(c,y);auto value=v<32768?std::to_string(v):"~"+std::to_string(65535-v);out+="        do Memory.poke(16384 + location + "+std::to_string(y*32+c)+", "+value+");\n";}
        return out+"        return;\n    }\n}\n";
    }
};
}
