#include <thread>
#include <atomic>
#include <Engine.h>

extern void OpenCV::LogMessage(const char* msg);

template <typename T>
bool IsValidT(const T* Test) {
	return ::IsValid(Test) && Test->IsValid();
}

namespace OpenCV {

class AsyncEngine {
public:
	void SetTask(std::function<void()>&& asyncF, std::function<void()>&& syncF) {
		if(!ensure(IsAvailable())) return;
		_syncF = std::move(syncF);
		_thread = std::thread([asF = std::move(asyncF), this]() { asF(); _done.store(true, std::memory_order_release); });
		_running = true;
	}

	void StopTask() {
		if(!_running) return;
		if(_done.load(std::memory_order_acquire)) {
			checkf(_thread.joinable(), L"Async thread not joinable");
			_thread.join();
			_running = false;
		} else {		// Unsalvagable: detach
			_thread.detach();
		}
	}

	void Tick() {
		if(_done.exchange(false, std::memory_order_acquire)) {
			checkf(_thread.joinable(), L"Async thread not joinable");
			_thread.join();
			_running = false;
			_syncF();
		}
	}

	bool IsTickable() const { return _running; }
	bool IsAvailable() const { return !_running; }

private:
	std::thread _thread;
	std::atomic<bool> _done;
	bool _running = false;

	std::function<void()> _syncF;
};

inline char* allocCString(const FString& str) {
	const auto cast = StringCast<char>(*str);
	char* res = (char*)FMemory::SystemMalloc(cast.Length() + 1);
	FMemory::Memcpy(res, cast.Get(), cast.Length());
	res[cast.Length()] = 0;
	return res;
}

inline void freeCString(const char* cstr) {
	FMemory::SystemFree(const_cast<char*>(cstr));
}

template <typename T1, typename... Ts>
inline bool _valid(T1* obj, Ts*... pack) {
	return IsValid(obj) && _valid(pack...);
}

template <typename Last>
inline bool _valid(Last* obj) {
	return IsValid(obj);
}

template <typename... ObjectsT>
inline bool checkValid(ObjectsT*... objects) {
	if(_valid(objects...)) {
		return true;
	} else {
		OpenCVLog("Trying to use uninitialized object");
		return false;
	}
}

}


