#pragma once

namespace Coral {

	enum class AssemblyLoadStatus
	{
		Success,
		FileNotFound,
		FileLoadFailure,
		InvalidFilePath,
		InvalidAssembly,
		UnknownError
	};

	class AssemblyHandle
	{
	public:
		uint16_t GetAssemblyID() const { return m_AssemblyID; }

	private:
		uint16_t m_AssemblyID;

		friend class HostInstance;
	};

	struct AssemblyData
	{
		std::string Name;
	};

}
