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
		int32_t GetAssemblyID() const { return m_AssemblyID; }

	private:
		int32_t m_AssemblyID;

		friend class HostInstance;
	};

	struct AssemblyData
	{
		std::string Name;
	};

}
