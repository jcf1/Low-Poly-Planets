/** \file NoiseGen.cpp */
#include "NoiseGen.h"

using namespace G3D;

void NoiseGen::generateMountainImage(shared_ptr<Image> image, float frequency, float multiplier){
    for(int x(0); x < image->width(); ++x){
        for(int y(0); y < image->height(); ++y) {
            Color1 col;
            image->get(Point2int32(x, y), col);
            Color1 col2 = multiplier * Color1(unorm8::fromBits(sampleUint8( (int) (x * frequency) << 12, (int) (y * frequency) <<12, 0)));
            image->set(x, y, col + col2);
        }
    }
}

void NoiseGen::generateLandImage(shared_ptr<Image> image, float frequency){
    for(int x(0); x < image->width(); ++x){
        for(int y(0); y < image->height(); ++y) {
            image->set(x, y, Color1unorm8(unorm8::fromBits(sampleUint8( (int) (x * frequency) << 12, (int) (y * frequency) <<12, 0))));
        }
    }
}

void NoiseGen::generateSeaImage(shared_ptr<Image> image, float frequency){
    for(int x(0); x < image->width(); ++x){
        for(int y(0); y < image->height(); ++y) {
            image->set(x, y, Color1unorm8(unorm8::fromBits(sampleUint8( (int) (x * frequency) << 12, (int) (y * frequency) <<12, 0))));
        }
    }
}

void NoiseGen::colorMountainImage(shared_ptr<Image> noise, shared_ptr<Image> colorMap) {
    for(int x(0); x < noise->width(); ++x){
        for(int y(0); y < noise->height(); ++y) {
            Color1unorm8 color;
            noise->get(Point2int32(x, y), color);
            if((float) color.value < 0.65f) {
                colorMap->set(x, y, Color3(1.0f));
            } else {
                colorMap->set(x, y, Color3(0.5f));
            }
        }
    }
}

void NoiseGen::landMapImage(const shared_ptr<Image>& land, const shared_ptr<Image>& mountain, shared_ptr<Image>& landMap, float oceanLevel, float power, float multiplier){
    for(int x(0); x < land->width(); ++x){
        for(int y(0); y < land->height(); ++y) {
            Color1unorm8 color;
            Point2int32 position(x, y);
            float landHeight;
            float mountHeight;

            land->get(position, color);
            if((1.0f - (float) color.value) > (oceanLevel + 0.1f)) {
                landHeight = 4.0f * (1.0f - (float) color.value);
                mountain->get(position, color);
                mountHeight = 1.0f - (float) color.value;
                mountHeight = 1.0f + pow(mountHeight, power) * multiplier;
                if(landHeight > mountHeight) {
                    landMap->set(x, y, Color3(1.0));
                }
            }
        }
    }
}

NoiseGen::NoiseGen() {
}