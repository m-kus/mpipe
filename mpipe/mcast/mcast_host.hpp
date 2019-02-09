#pragma once

#include <mcast/mcast_extension.hpp>

namespace mpipe
{
	class McastHost : public McastBook
	{
	protected:
		
		std::vector<McastExtension*> extensions_;

	public:

		McastHost(Security* security, const std::vector<McastExtension*>& extensions, std::int64_t tolerance = 0)
			: McastBook(security, tolerance)
			, extensions_(extensions)
		{
			for (auto& extension : extensions_)
				extension->host = this;
		}

		virtual ~McastHost() {}

		bool OnlineImpl() override
		{
			return std::all_of(extensions_.begin(), extensions_.end(), 
				[](McastExtension* ext) { return ext->Online(); }
			);
		}

		void ResetImpl(std::int64_t ts) override
		{
			for (auto& extension : extensions_)
				extension->Reset(ts);
		}
		
		void UpdateBegin(std::int64_t ts) override
		{
			for (auto& extension : extensions_)
				extension->UpdateBegin(ts);
		}

		void UpdateImpl(const McastBookUpdate& update) override
		{
			for (auto& extension : extensions_)
				extension->Update(update);
		}

		void UpdateEndImpl(std::int64_t ts) override
		{
			for (auto& extension : extensions_)
				extension->UpdateEnd(ts);
		}
	};
}
