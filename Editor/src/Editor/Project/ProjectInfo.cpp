#include "Editor/Project/ProjectInfo.h"
#include "Frost.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace Frost;

namespace fs = std::filesystem;

namespace Editor
{
	bool ProjectInfo::LoadFromPath(const std::string& path)
	{
		fs::path fsPath(path);

		if (fs::is_directory(fsPath))
		{
			for (const auto& entry : fs::directory_iterator(fsPath))
			{
				if (entry.path().extension() == ".frost")
				{
					_projectFilePath = entry.path().string();
					break;
				}
			}
		}
		else if (fsPath.extension() == ".frost")
		{
			_projectFilePath = path;
		}

		if (_projectFilePath.empty() || !fs::exists(_projectFilePath))
		{
			return false;
		}

		_projectDir = fs::path(_projectFilePath).parent_path().string();

		// Parse YAML
		try
		{
			YAML::Node data = YAML::LoadFile(_projectFilePath);
			if (data["Project"])
			{
				_config.name = data["project"]["name"].as<std::string>();
				_config.version = data["project"]["version"].as<std::string>();
				_config.startScene = data["project"]["startScene"].as<std::string>();
				_config.assetDirectory = data["project"]["assetDirectory"].as<std::string>();
				_config.sourceDirectory = data["project"]["sourceDirectory"].as<std::string>();
				return true;
			}
		}
		catch (YAML::ParserException& e)
		{
			std::cerr << "Failed to load project file: " << e.what() << std::endl;
		}

		return false;
	}

	bool ProjectInfo::CreateNewProject(const std::string& name, const std::string& parentDir, const std::string& templatePath)
	{
		fs::path targetDir = fs::path(parentDir) / name;
		fs::path templateDir(templatePath);

		if (fs::exists(targetDir))
		{
			FT_ENGINE_CRITICAL("Project directory '{}' already exists.", targetDir.string());
			return false;
		}

		if (!fs::exists(templateDir))
		{
			FT_ENGINE_CRITICAL("Template directory '{}' does not exist.", templateDir.string());
			return false;
		}

		try
		{
			fs::copy(templateDir, targetDir, fs::copy_options::recursive);

			fs::path projectFile = targetDir / "project.frost";

			if (fs::exists(targetDir / "template.frost"))
			{
				fs::rename(targetDir / "template.frost", projectFile);
			}

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "project" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "name" << YAML::Value << name;
			out << YAML::Key << "version" << YAML::Value << VERSION;
			out << YAML::Key << "startScene" << YAML::Value << "DefaultScene";
			out << YAML::Key << "assetDirectory" << YAML::Value << "resources";
			out << YAML::Key << "sourceDirectory" << YAML::Value << "src";
			out << YAML::EndMap;
			out << YAML::EndMap;

			std::ofstream fout(projectFile);
			fout << out.c_str();

			return true;
		}
		catch (std::exception& e)
		{
			std::cerr << "Error creating project: " << e.what() << std::endl;
			return false;
		}
	}
}