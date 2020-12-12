#include <iostream>
#include <vector>
#include <math.h>   
#include <cmath>
#include <map>
#include <cstdlib>
#include <bits/stdc++.h> 
#include <fstream>


using namespace std;

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

vector<float> VectorMath(vector<float> a,vector<float> b, string oper){
    if(oper == "subtract"){
        return vector<float>{a[0]-b[0],a[1]-b[1],a[2]-b[2]};
    }
    else if(oper == "add"){
        return vector<float>{a[0]+b[0],a[1]+b[1],a[2]+b[2]};
    }
    else if(oper == "multiply"){
        return vector<float>{a[1]*b[2] - a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
    }
    else if(oper == "rotate"){
        vector<float> out{a[0],a[1]*cos(b[0]) - a[2] * sin(b[0]) ,a[0]*sin(b[0]) - a[2] * cos(b[0])};
        out = {out[0]*cos(b[1]) + out[2] * sin(b[1]),out[1],-out[0] * sin(b[1]) + out[0] * cos(b[1])};
        return out;
    }
    else if(oper == "dotProduct"){
        return vector<float>{(a[0]*b[0])+(a[1]*b[1])+(a[2]*b[2])};
    }
    throw invalid_argument("unknown operation");
}

class material{
    public:
        string type = "matt";
        vector<int> color{255,255,255};
        float roughness = 0.5;
};

class tri{
    public:
        float k;
        vector<float> normal;
        vector<vector<float>> verts;

        void calcNormal(){
            vector<float> t1 = verts[0];
            vector<float> t2 = verts[1];
            vector<float> t3 = verts[2];
            vector<float> lineA = VectorMath(t2,t1,"subtract");
            vector<float> lineB = VectorMath(t3,t1,"subtract");
            normal = VectorMath(lineA,lineB,"multiply");
            calcK();
        };
        void calcK(){
            k = -(normal[0]*verts[0][0] + normal[1] * verts[0][1] + normal[2]*verts[0][2]);
        };
    
};

class obj{
    public:
        vector<tri> triangles;
        material mat;
};

class ray{
    public:
        vector<float> s{0,0,0};
        vector<float> m{0,0,1};
        float lambda = 0.0;
        float plane_intersect(tri triangle){
            float lambd = -(triangle.normal[0]*s[0] + triangle.normal[1]*s[1] + triangle.normal[2]*s[2]+triangle.k)/(triangle.normal[0]*m[0] + triangle.normal[1]*m[1] + triangle.normal[2]*m[2]);
            if(lambd <= 0){
                return NAN;
            }
            vector<float> i{lambd*m[0]+s[0],lambd*m[1]+s[1],lambd*m[2]+s[2]};
            if ((i[0] < min(min(triangle.verts[0][0],triangle.verts[1][0]),triangle.verts[2][0]) || i[0] > max(max(triangle.verts[0][0],triangle.verts[1][0]),triangle.verts[2][0])) || (i[1] < min(min(triangle.verts[0][1],triangle.verts[1][1]),triangle.verts[2][1]) || i[1] > max(max(triangle.verts[0][1],triangle.verts[1][1]),triangle.verts[2][1])) || (i[2] < min(min(triangle.verts[0][2],triangle.verts[1][2]),triangle.verts[2][2]) || i[2] > max(max(triangle.verts[0][2],triangle.verts[1][2]),triangle.verts[2][2]))){
                return NAN;
            }
            vector<float> v1 = VectorMath(triangle.verts[1],triangle.verts[2],"subtract");
            vector<float> a1 = VectorMath(v1,VectorMath(i,triangle.verts[2],"subtract"),"multiply");
            vector<float> b1 = VectorMath(v1,VectorMath(triangle.verts[0],triangle.verts[2],"subtract"),"multiply");
            float c1 = VectorMath(a1,b1,"dotProduct")[0];
            
            vector<float> v2 = VectorMath(triangle.verts[2],triangle.verts[0],"subtract");
            vector<float> a2 = VectorMath(v2,VectorMath(i,triangle.verts[0],"subtract"),"multiply");
            vector<float> b2 = VectorMath(v2,VectorMath(triangle.verts[1],triangle.verts[0],"subtract"),"multiply");
            float c2 = VectorMath(a2,b2,"dotProduct")[0];

            vector<float> v3 = VectorMath(triangle.verts[0],triangle.verts[1],"subtract");
            vector<float> a3 = VectorMath(v3,VectorMath(i,triangle.verts[1],"subtract"),"multiply");
            vector<float> b3 = VectorMath(v3,VectorMath(triangle.verts[2],triangle.verts[1],"subtract"),"multiply");
            float c3 = VectorMath(a3,b3,"dotProduct")[0];

            if (c3<0 && c2<0 && c1<0){
                return NAN;
            }

            float d = sqrt(pow((i[0]-s[0]),2)+pow((i[1]-s[1]),2)+pow((i[2]-s[2]),2));
            return d;

        }

};

class cam{
    public:
        vector<float> location{0,0,0};
        vector<float> rotation{0,0};
        vector<int> resolution{100,100};
        float pixelSize = 2/resolution[1];
        ray genarate_ray(int pitch,int yaw){
            vector<float> direction = rotation;
            direction[0] += pitch;
            direction[1] += yaw;
            ray ry;
            ry.s = location;
            ry.m = VectorMath(vector<float>{direction[0],direction[1],1},rotation,"rotate");
            return ry;
        }
};

class scene{
    public:
        vector<obj> objects;
        cam camera;
};

vector<vector<vector<int>>> render(scene scn){
    vector<vector<vector<int>>> image;
    for(int i=0; i<scn.camera.resolution[1]; i++){
        vector<vector<int>> row;
        for(int i2=0; i2<scn.camera.resolution[0]; i2++){
            row.push_back(vector<int>{0,0,0});
        }
        image.push_back(row);
    }
    for(int y =0; y<scn.camera.resolution[1];y++){
        for(int x =0; x<scn.camera.resolution[0];x++){
            ray ry = scn.camera.genarate_ray(x,y);
            vector<float> distances;
            for(int object=0;object<scn.objects.size();object++){
                for(int triangle=0;triangle<scn.objects[object].triangles.size();triangle++){
                    float dist = ry.plane_intersect(scn.objects[object].triangles[triangle]);
                    if (dist != NAN){
                        distances.push_back(dist);
                    }
                }
            }
            if (distances.size() == 0){
                continue;
            }
            sort(distances.begin(), distances.end());
            image[y][x] =/*  vector<int>{int(floor(255*distances[0])),int(floor(255*distances[0])),int(floor(255*distances[0]))} */ vector<int>{255,255,255};
        }
    }
    return image;
}

void saveRender(string filename, vector<vector<vector<int>>> render){
    image img(render[0].size(), render.size());
    for(int x=0; x<render[0].size(); x++){
        for (int y = 0; y < render.size(); y++)
        {
            img.r(x,y) = render[y][x][0];
            img.g(x,y) = render[y][x][1];
            img.b(x,y) = render[y][x][2];
        }
    }
    ofstream(filename) << img;
}

int main() {
    scene scn;
    tri tri0;
    tri0.verts.push_back(vector<float>{-0.1572267860174179,-0.11273674666881561,-0.09992553293704987});
    tri0.verts.push_back(vector<float>{-0.061701029539108276,-0.49193084239959717,-0.09992553293704987});
    tri0.verts.push_back(vector<float>{0.0,-0.27475789189338684,0.12357194721698761});
    tri0.calcNormal();
    obj object;
    object.triangles.push_back(tri0);
    scn.objects.push_back(object);
    cam camera;
    camera.location = vector<float>{0,-2.846518,0};
    camera.rotation = vector<float>{1,0};
    camera.resolution = vector<int>{200,200};
    scn.camera = camera;
    vector<vector<vector<int>>> finalImageRender = render(scn);
    saveRender("/tmp/renderTest.bmp",finalImageRender);
    return 0;
}