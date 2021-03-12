//CSCI 5607 HW3 - Rays & Files
//This HW has three steps:
// 1. Compile and run the program (the program takes a single command line argument)
// 2. Understand the code in this file (rayTrace_pga.cpp), in particular be sure to understand:
//     -How ray-sphere intersection works
//     -How the rays are being generated
//     -The pipeline from rays, to intersection, to pixel color
//    After you finish this step, and understand the math, take the HW quiz on canvas
// 3. Update the file parse_pga.h so that the function parseSceneFile() reads the passed in file
//     and sets the relevant global variables for the rest of the code to product to correct image

//To Compile: g++ -fsanitize=address -std=c++11 rayTrace_pga.cpp

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

//Images Lib includes:
#define STB_IMAGE_IMPLEMENTATION //only place once in one .cpp file
#define STB_IMAGE_WRITE_IMPLEMENTATION //only place once in one .cpp files
#include "image_lib.h" //Defines an image class and a color class

//#3D PGA
#include "PGA_3D.h"

//High resolution timer
#include <chrono>

#include <algorithm>

//Scene file parser
#include "parse_pga.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

bool raySphereIntersect_fast(Point3D rayStart, Line3D rayLine, Point3D sphereCenter, float sphereRadius){
  Dir3D dir = rayLine.dir();
  float a = dot(dir,dir);
  Dir3D toStart = (rayStart - sphereCenter);
  float b = 2 * dot(dir,toStart);
  float c = dot(toStart,toStart) - sphereRadius*sphereRadius;
  float discr = b*b - 4*a*c;
  if (discr < 0) return false;
  else{
    float t0 = (-b + sqrt(discr))/(2*a);
    float t1 = (-b - sqrt(discr))/(2*a);
    if (t0 > 0 || t1 > 0) return true;
  }
  return false;
}

Point3D raySphereIntersect(Point3D rayStart, Line3D rayLine, Point3D sphereCenter, float sphereRadius){
  Point3D projPoint = dot(rayLine,sphereCenter)*rayLine;      //Project to find closest point between circle center and line [proj(sphereCenter,rayLine);]
  float distSqr = projPoint.distToSqr(sphereCenter);          //Point-line distance (squared)
  float d2 = distSqr/(sphereRadius*sphereRadius);             //If distance is larger than radius, then...
  if (d2 > 1) return false;                                   //... the ray missed the sphere
  float w = sphereRadius*sqrt(1-d2);                          //Pythagorean theorem to determine dist between proj point and intersection points
  Point3D p1 = projPoint - rayLine.dir()*w;                   //Add/subtract above distance to find hit points
  Point3D p2 = projPoint + rayLine.dir()*w; 

  // if (dot((p1-rayStart),rayLine.dir()) >= 0) return (p1-rayStart).magnitude();     //Is the first point in same direction as the ray line?
  // if (dot((p2-rayStart),rayLine.dir()) >= 0) return (p2-rayStart).magnitude();     //Is the second point in same direction as the ray line?
  // return -1;
  if (dot((p1-rayStart),rayLine.dir()) >= 0) return p1;
  if (dot((p2-rayStart),rayLine.dir()) >= 0) return p2; 
  return Point3D(0,0,0); // no intersect
}

int main(int argc, char** argv){
  
  //Read command line paramaters to get scene file
  if (argc != 2){
     std::cout << "Usage: ./a.out scenefile\n";
     return(0);
  }
  std::string secenFileName = argv[1];

  //Parse Scene File
  parseSceneFile(secenFileName);

  float imgW = img_width, imgH = img_height;
  float halfW = imgW/2, halfH = imgH/2;
  float d = halfH / tanf(halfAngleVFOV * (M_PI / 180.0f));
  float r,g,b;

  Image outputImg = Image(img_width,img_height);
  auto t_start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < img_width; i++){
    for (int j = 0; j < img_height; j++){
      //TODO: In what way does this assumes the basis is orthonormal?
      float u = (halfW - (imgW)*((i+0.5)/imgW));
      float v = (halfH - (imgH)*((j+0.5)/imgH));
      Point3D p = eye - d*forward + u*right + v*up;
      Dir3D rayDir = (p - eye); 
      Line3D rayLine = vee(eye,rayDir).normalized();  //Normalizing here is optional
      
      bool hit = false;
      int hit_sphere = -1;
      float closest_dist = 99999, cur_dist = -1, n_dot_l, n_dot_h, i_src;
      Point3D cur_pt, closest_pt(999,999,999);
      Dir3D normal, to_light, to_eye, bisect;
      r=g=b=0;
      
      for (int s = 0; s < num_spheres; s++) {
        cur_pt = raySphereIntersect(eye,rayLine,sphere_array[s],radius_array[s]);
        
        if ( (cur_pt - Point3D(0,0,0)).magnitude() != 0) {
          cur_dist = (cur_pt - eye).magnitude();
          if (cur_dist > 0 && cur_dist < closest_dist) {
            hit = true;
            closest_dist = cur_dist;
            closest_pt = cur_pt;
            hit_sphere = s;
          }
        }        
      }

      Color color;
      bool shadow = false;
      if (hit) {
        normal = (closest_pt - sphere_array[hit_sphere]).normalized();
        
        for (int l = 0; l < num_lights; l++) {
          //shadow = false;
          
          // for (int s = 0; s < num_spheres; s++) {
            
          //   if ( (raySphereIntersect(closest_pt, vee(closest_pt, light_location_array[l] - closest_pt), sphere_array[s], radius_array[s]) - Point3D(0,0,0)).magnitude() != 0) {
          //     shadow = true;
          //     break;
          //   }
          // }
          
          if (!shadow) {
            if (light_type_array[l] == 0) { // Directional Light
              // diffuse lighting
              Dir3D dir_light = light_dir_array[l];
              to_light = Dir3D(-dir_light.x, -dir_light.y, -dir_light.z);
              n_dot_l = dot(to_light.normalized(), normal);
              if (n_dot_l > 0) {
                r += dif_color_array[hit_sphere].r * intensity_array[l].r * n_dot_l;
                g += dif_color_array[hit_sphere].g * intensity_array[l].g * n_dot_l;
                b += dif_color_array[hit_sphere].b * intensity_array[l].b * n_dot_l;
              }

              // Phong specularity
              to_eye = eye - closest_pt;
              bisect = (to_eye + to_light) / (to_eye + to_light).magnitude();
              n_dot_h = dot(normal, bisect);
              if (n_dot_h > 0) {
                r += spc_color_array[hit_sphere].r * intensity_array[l].r * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                g += spc_color_array[hit_sphere].g * intensity_array[l].g * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                b += spc_color_array[hit_sphere].b * intensity_array[l].b * pow(n_dot_h, spc_highlight_array[hit_sphere]);
              }
            } else if (light_type_array[l] == 1) { // Point Light
              // diffuse lighting
              to_light = (light_location_array[l] - closest_pt);
              n_dot_l = dot(to_light.normalized(), normal);
              if (n_dot_l > 0) {
                r += dif_color_array[hit_sphere].r * intensity_array[l].r / pow(to_light.magnitude(), 2.0) * n_dot_l;
                g += dif_color_array[hit_sphere].g * intensity_array[l].g / pow(to_light.magnitude(), 2.0) * n_dot_l;
                b += dif_color_array[hit_sphere].b * intensity_array[l].b / pow(to_light.magnitude(), 2.0) * n_dot_l;
              }

              // Phong specularity
              to_eye = eye - closest_pt;
              bisect = (to_eye + to_light) / (to_eye + to_light).magnitude();
              n_dot_h = dot(normal, bisect);
              if (n_dot_h > 0) {
                r += spc_color_array[hit_sphere].r * intensity_array[l].r / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                g += spc_color_array[hit_sphere].g * intensity_array[l].g / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                b += spc_color_array[hit_sphere].b * intensity_array[l].b / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
              }
            } else if (light_type_array[l] == 2) { // Spot Light
              // diffuse lighting
              to_light = (light_location_array[l] - closest_pt);
              float angle = acos(dot(-1 * to_light, light_dir_array[l]));
              float factor = 1;
              if (angle <= angle_array[2 * l]) {
                factor = 1;
              } else if (angle > angle_array[2 * l] && angle <= angle_array[2 * l + 1]) {
                float range = angle_array[2 * l + 1] - angle_array[2 * l];
                factor = (angle - angle_array[2 * l]) / range;
              } else {
                factor = 0;
              }
              n_dot_l = dot(to_light.normalized(), normal);
              if (n_dot_l > 0) {
                r += factor * dif_color_array[hit_sphere].r * intensity_array[l].r / pow(to_light.magnitude(), 2.0) * n_dot_l;
                g += factor * dif_color_array[hit_sphere].g * intensity_array[l].g / pow(to_light.magnitude(), 2.0) * n_dot_l;
                b += factor * dif_color_array[hit_sphere].b * intensity_array[l].b / pow(to_light.magnitude(), 2.0) * n_dot_l;
              }

              // Phong specularity
              to_eye = eye - closest_pt;
              bisect = (to_eye + to_light) / (to_eye + to_light).magnitude();
              n_dot_h = dot(normal, bisect);
              if (n_dot_h > 0) {
                r += factor * spc_color_array[hit_sphere].r * intensity_array[l].r / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                g += factor * spc_color_array[hit_sphere].g * intensity_array[l].g / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
                b += factor * spc_color_array[hit_sphere].b * intensity_array[l].b / pow(to_light.magnitude(), 2.0) * pow(n_dot_h, spc_highlight_array[hit_sphere]);
              }
            } else {
              printf("Error: light type %d not recognized", light_type_array[l]);
            }
          }
        }

        // ambient light
        r += amb_r*amb_color_array[hit_sphere].r;
        g += amb_g*amb_color_array[hit_sphere].g;
        b += amb_b*amb_color_array[hit_sphere].b;
        
        color = Color(r,g,b);        
      }
      else {
        color = Color(back_r,back_g,back_b);
      }
      outputImg.setPixel(i,j, color);
      //outputImg.setPixel(i,j, Color(fabs(i/imgW),fabs(j/imgH),fabs(0))); //TODO: Try this, what is it visualizing?
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  printf("Rendering took %.2f ms\n",std::chrono::duration<double, std::milli>(t_end-t_start).count());

  outputImg.write(imgName.c_str());
  return 0;
}
