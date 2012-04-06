//
// NotificationStrategy.h
//
// $Id: //poco/1.3/Foundation/include/Poco/NotificationStrategy.h#2 $
//
// Library: Foundation
// Package: Events
// Module:  NotificationStrategy
//
// Definition of the NotificationStrategy interface.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef  Foundation_NotificationStrategy_INCLUDED
#define  Foundation_NotificationStrategy_INCLUDED


#include "Poco/Foundation.h"


namespace Poco {


template <class TArgs, class TDelegate> 
class NotificationStrategy
	/// The interface that all notification strategies must implement.
{
public:
	NotificationStrategy()
	{
	}

	virtual ~NotificationStrategy()
	{
	}

	virtual void notify(const void* sender, TArgs& arguments) = 0;
		/// Sends a notification to all registered delegates,

	virtual void add(const TDelegate& pDelegate) = 0;
		/// Adds a delegate to the strategy, if the delegate is not yet present

	virtual void remove(const TDelegate& pDelegate) = 0;
		/// Removes a delegate from the strategy if found.

	virtual void clear() = 0;
		/// Removes all delegates from the strategy.

	virtual bool empty() const = 0;
		/// Returns false if the strategy contains at least one delegate.
};


} // namespace Poco


#endif
