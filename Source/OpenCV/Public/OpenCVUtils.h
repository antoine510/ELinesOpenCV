#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <array>

template <typename T>
bool IsValidT(const T* Test) {
	return ::IsValid(Test) && Test->IsValid();
}

namespace OpenCV {

class AsyncTask {
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
		} else {			// Unsalvagable: detach
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

class AsyncEngine {
public:
	void AddTask(std::function<void()>&& asyncF, std::function<void()>&& syncF) {

	}

	void Tick() {
		for(auto& task : _tasks) {
			task.Tick();
		}
	}

	bool IsTickable() const { return _running; }
	bool IsAvailable() const { return !_running; }

private:
	std::array<AsyncTask, 4> _tasks;

	bool _running = false;
};

}


