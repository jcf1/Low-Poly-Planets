#pragma once
#include <G3D/G3DAll.h>

/***/
class PathTracer {
    protected:

        shared_ptr<CubeMap> m_skybox;

        //Holds Tris for a Scene
        mutable TriTree m_triangles;

        //Number of Scattering Depths
        int m_numScatter;

        //Number of Paths per Pixel
        int m_pathsPPx;

        //Array of Lights in the Scene
        Array<shared_ptr<Light>> m_lights;

        //Fills in rays based on camera position
        void generateRayBuffer(Array<Ray>& rays, const shared_ptr<Camera>& cam, const Point2int32& vertex, const Rect2D& rect2D, int& width);

        //Adds emissive value of sufel at each pixel location
        void addEmissives(const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<Ray>& rays, const Array<Color3>& modBuffer, const shared_ptr<Image>& image, const Point2int32& vertex, int& width);

        //Adds Radience to pixels based on shade function
        void shadeImage(const shared_ptr<Image>& image, const Array<Biradiance3>& biradianceBuffer, const Array<Ray>& rayBuffer, const Array<Ray>& shadowRayBuffer, const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<bool>& lightShadowedBuffer, const Array<Color3>& modBuffer, const Point2int32& vertex, int& width);
        
        //Scatters rays using the surfel scatter function
        void scatterRays(const shared_ptr<Image>& image, const Array<Biradiance3>& biradianceBuffer, const Array<Ray>& shadowRayBuffer, const Array<shared_ptr<Surfel>>& surfelBuffer, const Array<bool>& lightShadowedBuffer, Array<Color3>& modBuffer, Array<Ray>& rayBuffer, const Point2int32& endPoint, int& width);
       
        //Calculates Biradience and shadow ray based on light from chooseLight
        void calculateDirectLight(const Array<shared_ptr<Surfel>>& surfelBuffer, Array<Biradiance3>& biradianceBuffer,  Array<Ray>& shadowRayBuffer, const Point2int32& endPoint, int& width);
        
        //Returns biradience and position of a light for a surfel using importance sampling
        Biradiance3 chooseLight(const shared_ptr<Surfel>& surfel, Point3& position);

        //Returns Radience for a pixel
        Radiance3 shade(const Ray& ray, const Ray& shadowRay, const shared_ptr<Surfel>& surfel, Biradiance3 bi);
        
    public:

        //Constructor
        PathTracer(const shared_ptr<Scene>& scene, const int& scatter, const int& paths);
        ~PathTracer();

        //Generates image of a Scene using Path Tracing
        void pathTrace(const shared_ptr<Scene>& scene, const shared_ptr<Camera>& cam, const shared_ptr<Image>& image);
};