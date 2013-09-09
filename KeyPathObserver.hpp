
#pragma once


namespace CConf {
	class Context;

	class KeyPathObserver {
	public:
		KeyPathObserver(Context &ctxt, KeyPath &keyPath);
		KeyPathObserver(Context &ctxt, std::string &keyPathStr);
		~KeyPathObserver();

		KeyPath& keyPath() {
			return _keyPath;
		}

		Context& context() {
			return _context;
		}

	protected:
		friend class Context;

		//	the context calls this method when the value the observer is observing has changed
		virtual void notify() {}

	private:
		Context &_context;
		KeyPath _keyPath;
	};
}
