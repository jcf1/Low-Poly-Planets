/** \file PathTracer.cpp */
#include "PathTracer.h"
// Written by Jose Rivas and John Freeman
// Based on Julia Goldman and Jose Rivas's Ray Tracer
// This parallalizes a lot of what was done previously and creates a path tracing algorithm from it.

void PathTracer::pathTrace(const shared_ptr<Scene>& scene, const shared_ptr<Camera>& cam, const shared_ptr<Image>& image){
    int imWidth = image->width();
    int imHeight = image->height();
    Rect2D rect2D(Vector2(imWidth - 1, imHeight - 1));
    Point2int32 finPoint(imWidth, imHeight);

    Array<Color3> modBuffer;
    Array<Ray> rayBuffer;
    Array<shared_ptr<Surfel>> surfelBuffer;
    Array<Biradiance3> biradianceBuffer;
    Array<Ray> shadowRayBuffer;
    Array<bool> lightShadowedBuffer;

    modBuffer.resize(imWidth*imHeight);
    rayBuffer.resize(imWidth*imHeight);
    surfelBuffer.resize(imWidth*imHeight);
    biradianceBuffer.resize(imWidth*imHeight);
    shadowRayBuffer.resize(imWidth*imHeight);
    lightShadowedBuffer.resize(imWidth*imHeight);

    for(int i(0); i < m_pathsPPx; ++i) {
        generateRayBuffer(rayBuffer, cam, finPoint, rect2D, imWidth);
        modBuffer.setAll(Color3(1.0f / float(m_pathsPPx)));

        for(int j(0); j < m_numScatter; ++j) {
            m_triangles.intersectRays(rayBuffer, surfelBuffer, G3D::TriTreeBase::COHERENT_RAY_HINT );
            
            addEmissives(surfelBuffer, rayBuffer, modBuffer, image, finPoint, imWidth);
            
            if(m_lights.size() > 0) {
                calculateDirectLight(surfelBuffer, biradianceBuffer, shadowRayBuffer, finPoint, imWidth);
                m_triangles.intersectRays(shadowRayBuffer, lightShadowedBuffer, TriTree::COHERENT_RAY_HINT | TriTree::DO_NOT_CULL_BACKFACES | TriTree::OCCLUSION_TEST_ONLY);
                shadeImage(image, biradianceBuffer, rayBuffer, shadowRayBuffer, surfelBuffer, lightShadowedBuffer, modBuffer, finPoint, imWidth);
            }

            if(j != m_numScatter-1) {
                scatterRays(image, biradianceBuffer, shadowRayBuffer, surfelBuffer, lightShadowedBuffer, modBuffer, rayBuffer, finPoint, imWidth);
            }
        }
        debugPrintf("%d paths out of %d.\n", i, m_pathsPPx);
    }
}

void PathTracer::generateRayBuffer(Array<Ray>& rays, const shared_ptr<Camera>& camera, const Point2int32& endPoint, const Rect2D& rect2D, int& width) {
    Thread::runConcurrently(Point2int32(0,0), endPoint, [&] (Point2int32& pixel) {
        rays[pixel.x + pixel.y * width] = camera -> worldRay(pixel.x + .5f, pixel.y + .5f, rect2D);
    });
}

void PathTracer::addEmissives(const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<Ray>& rays, const Array<Color3>& modBuffer, const shared_ptr<Image>& image, const Point2int32& endPoint, int& width){
    Thread::runConcurrently(Point2int32(0,0), endPoint, [&] (Point2int32& pixel) { 
        int index = pixel.x + pixel.y * width;
        const shared_ptr<Surfel> surfel = surfelBuffer[index];

        if(notNull(surfel)) {
            image->increment(pixel, modBuffer[index] * surfel -> emittedRadiance(-rays[index].direction()));
        } else {
            image->increment(pixel, Color3::black().lerp(Color3::blue(), rays[index].direction().y/10.0f) * modBuffer[index]);//m_skybox->bilinear(rays[index].direction()).rgb() * modBuffer[index]);
        }
    });
}

void PathTracer::calculateDirectLight(const Array<shared_ptr<Surfel>>& surfelBuffer, Array<Biradiance3>& biradianceBuffer,  Array<Ray>& shadowRayBuffer, const Point2int32& endPoint, int& width){
    Thread::runConcurrently(Point2int32(0, 0), endPoint, [&] (Point2int32& pixel) { 
        int index = pixel.x + pixel.y * width;
        const shared_ptr<Surfel> surfel = surfelBuffer[index];
        Ray fakeRay = Ray(Point3(1000, 1000, 1000), Vector3(0, 1, 0), 0.0f, 0.00000001f);

        if(notNull(surfel)) {
            Point3 position;
            biradianceBuffer[index] = chooseLight(surfel, position);
            shadowRayBuffer[index] = Ray::fromOriginAndDirection(position, (surfel->position - position).direction(), .0001f, (surfel->position - position).length() - .0002f);
        } else {
            biradianceBuffer[index] = Biradiance3(0,0,0);
            shadowRayBuffer[index] = fakeRay;
        }
    });
}

void PathTracer::shadeImage(const shared_ptr<Image>& image, const Array<Biradiance3>& biradianceBuffer, const Array<Ray>& rayBuffer, const Array<Ray>& shadowRayBuffer, const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<bool>& lightShadowedBuffer, const Array<Color3>& modBuffer, const Point2int32& endPoint, int& width){
    Thread::runConcurrently(Point2int32(0,0), endPoint, [&] (Point2int32& pixel) {
        int index = pixel.x + pixel.y * width;

        if(!lightShadowedBuffer[index]) {
            image->increment(pixel, shade(rayBuffer[index], shadowRayBuffer[index], surfelBuffer[index], biradianceBuffer[index]) * modBuffer[index]);
        }
    });
}

void PathTracer::scatterRays(const shared_ptr<Image>& image, const Array<Biradiance3>& biradianceBuffer, const Array<Ray>& shadowRayBuffer, const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<bool>& lightShadowedBuffer, Array<Color3>& modBuffer, Array<Ray>& rayBuffer, const Point2int32& endPoint, int& width){
    Thread::runConcurrently(Point2int32(0,0), endPoint, [&] (Point2int32& pixel) { 
        int index = pixel.x + pixel.y * width;
        Color3 weight;
        Vector3 w_after;
        const shared_ptr<Surfel> surfel = surfelBuffer[index];
        if(notNull(surfel)) {
            surfel->scatter(PathDirection::EYE_TO_SOURCE, -rayBuffer[index].direction(), true, Random::threadCommon(), weight, w_after);
            modBuffer[index] *= weight;
            rayBuffer[index] = Ray::fromOriginAndDirection(surfel->position + surfel->geometricNormal * .0001f * sign(w_after.dot(surfel->geometricNormal)), w_after);
        }
    });
}

Biradiance3 PathTracer::chooseLight(const shared_ptr<Surfel>& surfel, Point3& position) {
    
    debugAssert(m_lights.size() > 0);

    if(m_lights.size() == 1) {
        shared_ptr<Light> l = m_lights[0];
        position = l->position().xyz();
        return l->biradiance(surfel->position);
    }
        
    Biradiance total = 0.0f;
    for(int i = 0; i < m_lights.size(); ++i) {
        total += m_lights[i]->biradiance(surfel->position).sum();
    }
    float count = G3D::Random::threadCommon().uniform(0.0f, total - 0.0001f);

    int i = 0;
    for(; i < m_lights.size() && count > 0.0f; ++i) {
        count -= m_lights[i]->biradiance(surfel->position).sum();
    }

    const shared_ptr<Light>& light =  m_lights[min(i, m_lights.size() - 1)];
    position = light->position().xyz();
    Biradiance3 birad = light->biradiance(surfel->position);
    return birad * (total / max(0.00001f, birad.sum()));
}

Radiance3 PathTracer::shade(const Ray& ray, const Ray& shadowRay, const shared_ptr<Surfel>& surfel, Biradiance3 bi){
    if(!notNull(surfel)) {
        return Radiance3(.1, .1, .1);
    }

    Vector3& X = surfel->position;
    const Vector3& n = surfel->geometricNormal;

    Vector3 w_in = shadowRay.direction();
    const Color3& f = surfel->finiteScatteringDensity(w_in, -ray.direction());

    return (bi * f * abs(w_in.dot(n))) + surfel->reflectivity(Random::threadCommon()) * 0.05f;
}


PathTracer::PathTracer(const shared_ptr<Scene>& scene, const int& scatter, const int& paths){
    Array<shared_ptr<Surface>> sceneSurfaces;
    scene->onPose(sceneSurfaces);
    m_skybox = scene->skyboxAsCubeMap();
    m_numScatter = scatter;
    m_pathsPPx = paths;
    m_triangles.setContents(sceneSurfaces);
    m_lights = scene->lightingEnvironment().lightArray;
}

PathTracer::~PathTracer(){
}

