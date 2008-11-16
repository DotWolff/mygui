/*!
	@file
	@author		Albert Semenov
	@date		11/2007
	@module
*/

#include <list>

namespace delegates
{
	#define MYGUI_COMBINE(a,b)							MYGUI_COMBINE1(a,b)
	#define MYGUI_COMBINE1(a,b)						a##b

	#define MYGUI_I_DELEGATE							MYGUI_COMBINE(IDelegate, MYGUI_SUFFIX)
	#define MYGUI_C_STATIC_DELEGATE			MYGUI_COMBINE(CStaticDelegate, MYGUI_SUFFIX)
	#define MYGUI_C_METHOD_DELEGATE			MYGUI_COMBINE(CMethodDelegate, MYGUI_SUFFIX)
	#define MYGUI_C_DELEGATE							MYGUI_COMBINE(CDelegate, MYGUI_SUFFIX)
	#define MYGUI_C_MULTI_DELEGATE				MYGUI_COMBINE(CMultiDelegate, MYGUI_SUFFIX)

	// базовый класс всех делегатов
	MYGUI_TEMPLATE		MYGUI_TEMPLATE_PARAMS
	class MYGUI_I_DELEGATE
	{
	public:
		virtual ~MYGUI_I_DELEGATE() {}
		virtual void invoke(MYGUI_PARAMS) = 0;
		virtual bool compare(MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS* _delegate) = 0;
		virtual bool compare(IDelegateUnlink * _unlink) { return false; }
	};

	// делегат для статической функции
	MYGUI_TEMPLATE		MYGUI_TEMPLATE_PARAMS
	class MYGUI_C_STATIC_DELEGATE : public MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS
	{
	public:
		typedef void (*Func)(MYGUI_PARAMS);

		MYGUI_C_STATIC_DELEGATE(Func _func) : mFunc(_func) { }

		virtual void invoke(MYGUI_PARAMS)
		{
			mFunc(MYGUI_ARGS);
		}

		virtual bool compare(MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS* _delegate)
		{
			if (!_delegate || typeid(*this) != typeid(*_delegate)) return false;
			return (static_cast<MYGUI_C_STATIC_DELEGATE	MYGUI_TEMPLATE_ARGS*>(_delegate)->mFunc != mFunc);
		}

	private:
		Func mFunc;
	};

	// делегат для метода класса
	template MYGUI_T_TEMPLATE_PARAMS
	class MYGUI_C_METHOD_DELEGATE : public MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS
	{
	public:
		typedef void (T::*Method)(MYGUI_PARAMS);

		MYGUI_C_METHOD_DELEGATE(IDelegateUnlink * _unlink, T * _object, Method _method) :
			mUnlink(_unlink),
			mObject(_object),
			mMethod(_method)
		{
		}

		virtual void invoke(MYGUI_PARAMS)
		{
			(mObject->*mMethod)(MYGUI_ARGS);
		}

		virtual bool compare(MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS* _delegate)
		{
			if (!_delegate || typeid(*this) != typeid(*_delegate)) return false;
			MYGUI_C_METHOD_DELEGATE		MYGUI_T_TEMPLATE_ARGS* cast =
				static_cast<MYGUI_C_METHOD_DELEGATE		MYGUI_T_TEMPLATE_ARGS*>(_delegate);
			if ( cast->mObject != mObject || cast->mMethod != mMethod ) return false;
			return true;
		}

		virtual bool compare(IDelegateUnlink * _unlink)
		{
			return mUnlink == _unlink;
		}

	private:
		IDelegateUnlink *mUnlink;
		T * mObject;
		Method mMethod;
	};


	// шаблон для создания делегата статической функции
	// параметры : указатель на функцию
	// пример : newDelegate(funk_name);
	// пример : newDelegate(class_name::static_method_name);
	MYGUI_TEMPLATE		MYGUI_TEMPLATE_PARAMS
	inline MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS* newDelegate(void (*_func)(MYGUI_PARAMS))
	{
		return new MYGUI_C_STATIC_DELEGATE		 MYGUI_TEMPLATE_ARGS(_func);
	}

	// шаблон для создания делегата метода класса
	// параметры : указатель на объект класса и указатель на метод класса
	// пример : newDelegate(&object_name, &class_name::method_name);
	template MYGUI_T_TEMPLATE_PARAMS
	inline MYGUI_I_DELEGATE		MYGUI_TEMPLATE_ARGS* newDelegate(T * _object, void (T::*_method)(MYGUI_PARAMS))
	{
		return new MYGUI_C_METHOD_DELEGATE	 MYGUI_T_TEMPLATE_ARGS (GetDelegateUnlink(_object), _object, _method);
	}

	// шаблон класса делегата
	MYGUI_TEMPLATE	 MYGUI_TEMPLATE_PARAMS
	class MYGUI_C_DELEGATE
	{
	public:
		typedef MYGUI_I_DELEGATE	MYGUI_TEMPLATE_ARGS IDelegate;

		MYGUI_C_DELEGATE() :
			mDelegate(0)
		{
		}

		~MYGUI_C_DELEGATE()
		{
			clear();
		}

		bool empty()
		{
			return mDelegate == 0;
		}

		void clear()
		{
			if (mDelegate) {
				delete mDelegate;
				mDelegate = 0;
			}
		}

		MYGUI_C_DELEGATE	MYGUI_TEMPLATE_ARGS& operator=(IDelegate* _delegate)
		{
			if (mDelegate) {
				delete mDelegate;
			}
			mDelegate = _delegate;
			return *this;
		}

		void operator()(MYGUI_PARAMS)
		{
			if (mDelegate == 0) return;
			mDelegate->invoke(MYGUI_ARGS);
		}

	private:
		IDelegate * mDelegate;
	};

	// шаблон класса мульти делегата
	MYGUI_TEMPLATE		MYGUI_TEMPLATE_PARAMS
	class MYGUI_C_MULTI_DELEGATE
	{
	public:
		typedef MYGUI_I_DELEGATE	MYGUI_TEMPLATE_ARGS IDelegate;

		MYGUI_C_MULTI_DELEGATE()
		{
		}

		~MYGUI_C_MULTI_DELEGATE()
		{
			clear();
		}

		bool empty()
		{
			std::list<IDelegate *>::iterator iter;
			for (iter=mListDelegates.begin(); iter!=mListDelegates.end(); ++iter) {
				if (*iter) return false;
			}
			return true;
		}

		void clear()
		{
			std::list<IDelegate *>::iterator iter;
			for (iter=mListDelegates.begin(); iter!=mListDelegates.end(); ++iter) {
				if (*iter) {
					delete (*iter);
					(*iter) = 0;
				}
			}
		}

		void clear(IDelegateUnlink * _unlink)
		{
			std::list<IDelegate *>::iterator iter;
			for (iter=mListDelegates.begin(); iter!=mListDelegates.end(); ++iter) {
				if ((*iter) && (*iter)->compare(_unlink)) {
					delete (*iter);
					(*iter) = 0;
				}
			}
		}

		MYGUI_C_MULTI_DELEGATE		MYGUI_TEMPLATE_ARGS& operator+=(IDelegate* _delegate)
		{
			std::list<IDelegate *>::iterator iter;
			for (iter=mListDelegates.begin(); iter!=mListDelegates.end(); ++iter) {
				if ((*iter) && (*iter)->compare(_delegate)) {
                                  assert("dublicate delegate");
				}
			}
			mListDelegates.push_back(_delegate);
			return *this;
		}

		MYGUI_C_MULTI_DELEGATE		MYGUI_TEMPLATE_ARGS& operator-=(IDelegate* _delegate)
		{
			std::list<IDelegate *>::iterator iter;
			for (iter=mListDelegates.begin(); iter!=mListDelegates.end(); ++iter) {
				if ((*iter) && (*iter)->compare(_delegate)) {
					// проверяем на идентичность делегатов
					if ((*iter) != _delegate) delete (*iter);
					(*iter) = 0;
					break;
				}
			}
			delete _delegate;
			return *this;
		}

		void operator()(MYGUI_PARAMS)
		{
			std::list<IDelegate *>::iterator iter=mListDelegates.begin();
			while (iter != mListDelegates.end()) {
				if (0 == (*iter)) iter = mListDelegates.erase(iter);
				else {
					(*iter)->invoke(MYGUI_ARGS);
					++iter;
				}
			};
		}

	private:
		std::list<IDelegate *> mListDelegates;
	};

	#undef MYGUI_COMBINE
	#undef MYGUI_COMBINE1

	#undef MYGUI_I_DELEGATE
	#undef MYGUI_C_STATIC_DELEGATE
	#undef MYGUI_C_METHOD_DELEGATE
	#undef MYGUI_C_DELEGATE
	#undef MYGUI_C_MULTI_DELEGATE

	#undef MYGUI_SUFFIX
	#undef MYGUI_TEMPLATE
	#undef MYGUI_TEMPLATE_PARAMS
	#undef MYGUI_TEMPLATE_ARGS
	#undef MYGUI_T_TEMPLATE_PARAMS
	#undef MYGUI_T_TEMPLATE_ARGS
	#undef MYGUI_PARAMS
	#undef MYGUI_ARGS

} // namespace delegates
