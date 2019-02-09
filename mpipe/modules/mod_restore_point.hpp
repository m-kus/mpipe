#pragma once

#include <core/mpipe.hpp>
#include <util/path.hpp>
#include <util/numeric.hpp>

#undef min
#undef max
#include <fast-cpp-csv-parser/csv.h>

#include <sstream>
#include <fstream>

namespace mpipe
{
	class ModRestorePoint : public Module
	{
	private:

		std::string restore_point_path_;

	protected:

		bool LinkPosition(
			State* state,
			const std::string& class_code,
			const std::string& full_code,
			std::int64_t position)
		{
			for (auto& sec : state->securities)
			{
				if (sec->class_code == class_code
					&& sec->full_code == full_code
					&& sec->leg_factor != 0)
				{
					sec->pos.position = position;
					sec->pos.ChunksApply(time_now(), npabs(position), (Side)npsign(position));
					return true;
				}
			}

			return false;
		}

		void LoadFromFile(const std::string& path)
		{
			io::CSVReader<3> file(path);

			auto state = GetState();
			std::string class_code;
			std::string full_code;
			std::int64_t position = 0;

			while (file.read_row(class_code, full_code, position))
			{
				log_format("%s %s: %lld", class_code.c_str(), full_code.c_str(), position);
				if (!LinkPosition(state, class_code, full_code, position))
					log_format("according security not found, beware");
			}
		}

	public:

		ModRestorePoint(const std::string& base_dir)
			: restore_point_path_(base_dir + "\\restore_point.csv")
		{
			DirectoriesCreate(base_dir);
		}

		void Mutate(State* state) override
		{
			std::stringstream ss;
			for (auto& sec : state->securities)
			{
				if (sec->leg_factor != 0)
				{
					ss << sec->class_code << ",";
					ss << sec->full_code << ",";
					ss << sec->pos.position << std::endl;
				}
			}

			std::ofstream file(restore_point_path_);
			file << ss.str();
		}

		void StartImpl() override
		{
			ScopedState state(GetState());
			LoadFromFile(restore_point_path_);
		}
	};
}
