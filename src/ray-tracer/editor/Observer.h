#pragma once

namespace CHR
{
	class Observer
	{
	public:
		virtual void GetNotified() = 0;
	};

	class Subject
	{
	public:
		virtual void Attach(Observer* observer) = 0;
		virtual void Detach(Observer* observer) = 0;
		virtual void Notify() = 0;
	};
};