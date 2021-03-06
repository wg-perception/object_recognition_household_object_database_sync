/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

// Author(s): Matei Ciocarlie

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


#include "household_objects_database/objects_database.h"
#include "household_objects_database/database_original_model.h"
#include "household_objects_database/database_scaled_model.h"
#include "household_objects_database/database_file_path.h"

#include "mesh_loader.h"



#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <fstream>


//hide the local functions in an unnamed namespace
namespace
{

namespace fs = boost::filesystem;
namespace po = boost::program_options;
struct Options
{
  std::string object_name;
  std::string object_id;
  std::string object_description;
  std::string geometry_file, thumbnail_file;
  std::string method;
  std::vector<std::string> tags;
};

int options(int ac, char ** av, Options& opts)
{
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()("help", "Produce help message.");
  desc.add_options()("geometry,G", po::value<std::string>(&opts.geometry_file)->default_value("mesh.ply"),
                     "Mesh file in binary ply format.");
  desc.add_options()("thumbnail,T", po::value<std::string>(&opts.thumbnail_file)->default_value("thumb.jpg"),
                     "A thumbnail file of the object.");
  desc.add_options()("method,M", po::value<std::string>(&opts.method)->default_value(""),
                     "The object name.");
  desc.add_options()("name,N", po::value<std::string>(&opts.object_name)->default_value(""),
                     "The object name.");
  desc.add_options()("description,D", po::value<std::string>(&opts.object_description)->default_value(""),
                     "A description of the object.");
  desc.add_options()("object_id,I",po::value<std::string>(&opts.object_id)->default_value(""),"The UID of the object in the object_recognition database.");
  desc.add_options()("tags", po::value<std::vector<std::string> >(&opts.tags), "The tags to associate with the object.");

  po::positional_options_description p;
  p.add("tags", -1);
  po::variables_map vm;
  po::store(po::command_line_parser(ac, av). options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 1;
  }
  if (opts.object_name.empty())
  {
    std::cout << "You must supply an object name." << std::endl;
    return 1;
  }
  if (opts.method.empty())
  {
    std::cout << "You must supply a method." << std::endl;
    return 1;
  }
  if (opts.object_description.empty())
  {
    //std::cout << "You must supply an object description." << std::endl;
    //return 1;
    opts.object_description = opts.object_name;
  }
  return 0;
}
}

std::string getNonEmptyString(const std::string &display_name)
{
  std::string ret_string;
  while(1)
  {
    std::cout << "Enter " << display_name << ": ";
    std::getline(std::cin, ret_string);
    if (!ret_string.empty()) break;
    std::cout << "Non-empty string required \n";
  }
  return ret_string;
}

std::string getString(const std::string &display_name)
{
  std::string ret_string;
  std::cout << "Enter " << display_name << " (leave empty for none):";
  std::getline(std::cin, ret_string);
  return ret_string;
}

bool copyFile(std::string old_path, std::string new_root, std::string &filename)
{
  //get just the filename part of the original file
  size_t slash_position = old_path.find_last_of('/');
  if (slash_position == std::string::npos) 
  {
    //no slash
    filename = old_path;
  } 
  else if (slash_position >= old_path.size() - 1)
  {
    //slash is the last character
    std::cerr << "Failed to parse input filename: " << old_path << "\n";
    return false;
  }
  else
  {
    //skip the last slash
    filename = old_path;//.replace('
    replace(filename.begin(), filename.end(), '/', '_');

    std::cout << "file: " << filename << std::endl;
  }
  std::string new_path(new_root);
  if (new_path.at(new_path.size() - 1) != '/') 
  {
    new_path.append("//");
  }
  new_path.append(filename);
  if ( boost::filesystem::exists(new_path) )
  {
    std::cerr << "File " << new_path << " already exists; skipping copy.\n";
    return true;
  }  
  boost::filesystem::copy_file(old_path, new_path);
  if ( !boost::filesystem::exists(new_path) )
  {
    std::cerr << "Failed to copy file " << filename << " to " << new_path << "\n";
    return false;
  }
  
  return true;
}

bool loadGeometry(household_objects_database::DatabaseMesh &mesh, std::string filename)
{
  return true;
}

void getOriginalModelInfo(household_objects_database::DatabaseOriginalModel &original_model, Options opts)
{
  std::vector<std::string> tags;
  tags = opts.tags;

  //hard-coded for TOD objects
  original_model.source_.data() = "Household";
  original_model.acquisition_method_.data() = opts.method;
  original_model.maker_.data() = "Unknown";
  original_model.model_.data() = opts.object_name;
  original_model.recognition_id_.data() = opts.object_id;
  original_model.barcode_.get() = "";
  original_model.description_.get() = opts.object_description;

  if (!tags.empty())
  {
    original_model.tags_.data() = tags;
    original_model.tags_.setWriteToDatabase(true);
  }
  if (!original_model.barcode_.get().empty())
  {
    original_model.barcode_.setWriteToDatabase(true);
  }
  if (!original_model.description_.get().empty()) 
  {
    original_model.description_.setWriteToDatabase(true);
  }
  if (!original_model.recognition_id_.get().empty())
  {
    original_model.recognition_id_.setWriteToDatabase(true);
  }

}

int main(int argc, char **argv)
{
  //connect to database
  //hard-coded for now
  household_objects_database::ObjectsDatabase database("wgs36", "5432", "willow", "willow", "household_objects");
  if (!database.isConnected())
  {
    std::cerr << "Database failed to connect";
    return -1;
  }

  Options opts;
  if( 0 != options(argc,argv,opts) )
    return 0;

  //check that the geometry file exists
  std::string geometry_filename(opts.geometry_file);
  if ( !boost::filesystem::exists(geometry_filename) )
  {
    std::cerr << "Geometry file " << geometry_filename << " not found\n";
    return -1;
  }

  //check that thumbnail file exists 
  std::string thumbnail_filename(opts.thumbnail_file);
  if ( !boost::filesystem::exists(thumbnail_filename) )
  {
    std::cout << "Thumbnail file " << thumbnail_filename << " not found\n";
    return -1;
  }

  //read in the geometry of the model from the ply file
  household_objects_database::DatabaseMesh mesh;
  household_objects_database::PLYModelLoader loader;
  if (loader.readFromFile(geometry_filename, mesh.vertices_.data(), mesh.triangles_.data()) < 0)
  {
    std::cerr << "Failed to read geometry from file\n";
    return -1;
  }
  if (mesh.vertices_.data().empty() || mesh.triangles_.data().empty())
  {
    std::cerr << "No geometry read from file\n";
    return -1;
  }
  
  //the original model we will insert
  household_objects_database::DatabaseOriginalModel original_model;
  //ask use for info to populate fields 
  getOriginalModelInfo(original_model,opts);

  //copy the file(s) over to the model root
  std::string model_root;
  if (!database.getModelRoot(model_root) || model_root.empty())
  {
    std::cerr << "Failed to get model root from database\n";
    return -1;
  }
  std::string geometry_relative_filename;
  if (!copyFile(geometry_filename, model_root, geometry_relative_filename)) return -1;
  std::string thumbnail_relative_filename;
  if (!copyFile(thumbnail_filename, model_root, thumbnail_relative_filename)) return -1;

  //insert the original model into the database
  if (!database.insertIntoDatabase(&original_model))
  {
    std::cerr << "Failed to insert original model in database\n";
    return -1;
  }
  int original_model_id = original_model.id_.data();

  //insert the geometry
  mesh.id_.data() = original_model_id;
  if (!database.insertIntoDatabase(&mesh))
  {
    std::cerr << "Failed to insert mesh in database\n";
    return -1;
  }
  if (!database.saveToDatabase(&mesh.triangles_) || !database.saveToDatabase(&mesh.vertices_))
  {
    std::cerr << "Failed to write mesh data to database\n";
    return -1;
  }

  // insert a scaled model at range 1.0
  household_objects_database::DatabaseScaledModel scaled_model;
  scaled_model.original_model_id_.data() = original_model_id;
  scaled_model.scale_.data() = 1.0;
  if (!database.insertIntoDatabase(&scaled_model))
  {
    std::cerr << "Failed to insert scaled model in database\n";
    return -1;
  }
  
  //insert the paths to the files in the database
  household_objects_database::DatabaseFilePath geometry_file_path;
  geometry_file_path.original_model_id_.data() = original_model_id;
  geometry_file_path.file_type_.data() = "GEOMETRY_BINARY_PLY";
  geometry_file_path.file_path_.data() = geometry_relative_filename;
  if (!database.insertIntoDatabase(&geometry_file_path))
  {
    std::cerr << "Failed to insert geometry file path in database\n";
    return -1;
  }  
  household_objects_database::DatabaseFilePath thumbnail_file_path;
  thumbnail_file_path.original_model_id_.data() = original_model_id;
  thumbnail_file_path.file_type_.data() = "THUMBNAIL_BINARY_PNG";
  thumbnail_file_path.file_path_.data() = thumbnail_relative_filename;
  if (!database.insertIntoDatabase(&thumbnail_file_path))
  {
    std::cerr << "Failed to insert thumbnail file path in database\n";
    return -1;
  }

  std::cerr << "Insertion succeeded\n";
  return 0;
}
