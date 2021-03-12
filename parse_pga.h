
//Set the global scene parameter variables
//TODO: Set the scene parameters based on the values in the scene file

#ifndef PARSE_PGA_H
#define PARSE_PGA_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

//Camera & Scene Parmaters (Global Variables)
//Here we set default values, override them in parseSceneFile()

//Image Parmaters
int img_width = 640, img_height = 480;
std::string imgName = "raytraced.bmp";

//Camera Parmaters
Point3D eye = Point3D(0,0,0); 
Dir3D forward = Dir3D(0,0,-1).normalized();
Dir3D up = Dir3D(0,1,0).normalized();
Dir3D right = Dir3D(-1,0,0).normalized();
float halfAngleVFOV = 45; 

//Scene (Sphere/background) Parmaters
float back_r = 0, back_b = 0, back_g = 0;
int max_depth = 5;
int num_spheres = 0;
Point3D sphere_array[20];
float radius_array[20];
Color amb_color_array[20];
Color dif_color_array[20];
Color spc_color_array[20];
Color trn_color_array[20];
float spc_highlight_array[20];
float ior_array[20];

// Material parameters
float ar = 0, ag = 0, ab = 0;
float dr = 1, dg = 1,db = 1;
float sr = 0, sg = 0, sb = 0, ns = 5;
float tr = 0, tg = 0, tb = 0, ior = 1;

// Directional Light
float dir_r = 0, dir_g = 0, dir_b = 0;
float dir_x = 0, dir_y = 0, dir_z = 0;

// Point Light
float pt_r = 0, pt_g = 0, pt_b = 0;
float pt_x = 0, pt_y = 0, pt_z = 0;

// Spot Light
float spot_r = 0, spot_g = 0, spot_b = 0;
float spot_loc_x = 0, spot_loc_y = 0, spot_loc_z = 0;
float spot_dir_x = 0, spot_dir_y = 0, spot_dir_z = 0;
float angle1 = 0, angle2 = 0;

// Light arrays
int light_type_array[20];
Color intensity_array[20];
Point3D light_location_array[20];
Dir3D light_dir_array[20];
int num_lights = 0;
float angle_array[40];

// Ambient Light
float amb_r = 0, amb_g = 0, amb_b = 0;

void parseSceneFile(std::string fileName){
  std::ifstream input(fileName.c_str());

  // check for errors in opening the file
  if(input.fail()){
    std::cout << "Can't open file '" << fileName << "'" << std::endl;
    return;
  }
  
  std::string command, line;
  while(input >> command) { //Read first word in the line (i.e., the command type)
    
    if (command[0] == '#'){
      getline(input, line); //skip rest of line
      //std::cout << "Skipping comment: " << command  << line <<  std::endl;
      continue;
    }
    
    
    if (command == "sphere:"){ //If the command is a sphere command
      float x,y,z,r;
      input >> x >> y >> z >> r;
      sphere_array[num_spheres] = Point3D(x,y,z);
      radius_array[num_spheres] = r;
      amb_color_array[num_spheres] = Color(ar,ag,ab);
      dif_color_array[num_spheres] = Color(dr,dg,db);
      spc_color_array[num_spheres] = Color(sr,sg,sb);
      trn_color_array[num_spheres] = Color(tr,tg,tb);
      spc_highlight_array[num_spheres] = ns;
      ior_array[num_spheres] = ior;
      printf("Sphere as position (%f,%f,%f) with radius %f\n",x,y,z,r);
      num_spheres++;
    }
    
    else if (command == "image_resolution:"){ //If the command is a image_resolution command
      input >> img_width >> img_height;
      printf("Image resolution of width %d x height %d\n", img_width, img_height);
    }
    
    else if (command == "output_image:"){ //If the command is an output_image command
      input >> imgName;
      printf("Render to file named: %s\n", imgName.c_str());
    }
    
    else if (command == "camera_pos:"){ //If the command is a camera_pos command
      float x,y,z;
      input >> x >> y >> z;
      eye = Point3D(x,y,z);
    }
    
    else if (command == "camera_fwd:"){ //If the command is an camera_fwd command
      float fx,fy,fz;
      input >> fx >> fy >> fz;
      forward = Dir3D(fx,fy,fz);
    }
    
    else if (command == "camera_up:"){ //If the command is a camera_up command
      float ux,uy,uz;
      input >> ux >> uy >> uz;
      up = Dir3D(ux,uy,uz);
    }
    
    else if (command == "camera_fov_ha:"){ //If the command is an camera_fov_ha command
      input >> halfAngleVFOV;
    }

    else if (command == "background:"){ //If the command is a background command
      input >> back_r >> back_g >> back_b;
    }

    else if (command == "material:"){ //If the command is a material command
      input >> ar >> ag >> ab >> dr >> dg >> db >> sr >> sg >> sb >> ns >> tr >> tg >> tb >> ior;
    }
    
    else if (command == "directional_light:"){ //If the command is a directional light command
      input >> dir_r >> dir_g >> dir_b >> dir_x >> dir_y >> dir_z;
      light_type_array[num_lights] = 0;
      intensity_array[num_lights] = Color(dir_r, dir_g, dir_b);
      light_dir_array[num_lights] = Dir3D(dir_x, dir_y, dir_z);
      num_lights++;
      printf("Directional light in dir (%f, %f, %f) with intensity (%f, %f, %f)\n", dir_x, dir_y, dir_z, dir_r, dir_g, dir_b);
    }

    else if (command == "point_light:"){ //If the command is a point light command
      input >> pt_r >> pt_g >> pt_b >> pt_x >> pt_y >> pt_z;
      light_type_array[num_lights] = 1;
      intensity_array[num_lights] = Color(pt_r, pt_g, pt_b);
      light_location_array[num_lights] = Point3D(pt_x, pt_y, pt_z);
      num_lights++;
      printf("Point light at (%f, %f, %f) with intensity (%f, %f, %f)\n", pt_x, pt_y, pt_z, pt_r, pt_g, pt_b);
    }

    else if (command == "spot_light:"){ //If the command is a spot light command
      input >> spot_r >> spot_g >> spot_b >> spot_loc_x >> spot_loc_y >> spot_loc_z >> spot_dir_x >> spot_dir_y >> spot_dir_z >> angle1 >> angle2;
      light_type_array[num_lights] = 2;
      intensity_array[num_lights] = Color(spot_r, spot_g, spot_b);
      light_location_array[num_lights] = Point3D(spot_loc_x, spot_loc_y, spot_loc_z);
      light_dir_array[num_lights] = Dir3D(spot_dir_x, spot_dir_y, spot_dir_z);
      angle_array[2 * num_lights] = angle1;
      angle_array[2 * num_lights + 1] = angle2;
      num_lights++;
      printf("Spot light at (%f, %f, %f) pointing in dir (%f, %f, %f) with intensity (%f, %f, %f)\n", spot_loc_x, spot_loc_y, spot_loc_z, spot_dir_x, spot_dir_y, spot_dir_z, spot_r, spot_g, spot_b);
    }

    else if (command == "ambient_light:"){ //If the command is a ambient light command
      input >> amb_r >> amb_g >> amb_b;
    }

    else if (command == "max_depth:"){ //If the command is a max depth command
      input >> max_depth;
    }
    
    else {
      getline(input, line); //skip rest of line
      std::cout << "WARNING. Unknown command: " << command << std::endl;
    }
  }

  

  right = cross(up, forward).normalized();
  forward = forward.normalized();
  up = up.normalized();

  if (! (dot(forward, up) == 0 && dot(right, up) == 0)) {
    up = cross(forward, right).normalized();
  }

  assert(dot(right, forward) == 0 && dot(right, up) == 0 && dot(forward, up) == 0);

  // printf("Orthagonal Camera Basis:\n");
  // forward.print("forward");
  // right.print("right");
  // up.print("up");
}

#endif