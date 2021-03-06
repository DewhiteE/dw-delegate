#pragma once

#include <vector>
#include <algorithm>
#include <tuple>
#include <iostream>
#include <functional>

namespace dw
{
    template <typename ReturnType, typename... Params>
    class SimpleDelegateBase
    {
    protected:
        /**
         * @brief           Type defining a pointer to the function with the same arguments as Delegate's
         */
        typedef ReturnType (*FunctionType)(Params...);
        //using FunctionType = std::function<ReturnType(Params...)>;

        /**
         * @brief           **std::vector** of functions that are subscribed to this delegate.
         */
        std::vector<FunctionType> subscribers;

        SimpleDelegateBase() = default;
    };

    template <typename... Params>
    class SimpleDelegate : public SimpleDelegateBase<void, Params...>
    {
    public:
        using Parent = SimpleDelegateBase<void, Params...>;
        using Parent::subscribers;
        using typename Parent::FunctionType;

        /**
         * @brief           Invoke all subscribed functions.
         * 
         * @param  params:  Arguments of each subscribed function.
         */
        void operator()(Params... params)
        {
            for (auto &&i : subscribers)
            {
                i(params...);
            }
        }

        /**
         * @brief           Subscribe function to this delegate.
         * 
         * @param  rhs:     Function to subscribe.
         * @returns         Reference to the delegate instance.
         */
        SimpleDelegate &operator+=(const FunctionType &rhs)
        {
            this->subscribers.push_back(rhs);
            return *this;
        }

        /**
         * @brief           Unsubscribe choosen function from this delegate.
         * 
         * @param  rhs:     Function to unsubscribe from this delegate.
         * @returns         Reference to the delegate instance.
         */
        SimpleDelegate &operator-=(const FunctionType &rhs)
        {
            subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), rhs), subscribers.end());
            return *this;
        }
    };

    template <typename ReturnType, typename... Params>
    class DelegateBase : public SimpleDelegateBase<ReturnType, Params...>
    {
        template <typename... T>
        struct FunctionParams
        {
            size_t index = -1;
            std::tuple<T...> parameters;
        };

    protected:
        using Parent = SimpleDelegateBase<ReturnType, Params...>;
        using Parent::subscribers;
        using typename Parent::FunctionType;

        /**
         * @brief           Vector of parameters updated when each function is subscribed to this delegate using Subscribe() method.
         */
        std::vector<FunctionParams<Params...>> parameters;

    public:
        const std::vector<FunctionType> &GetSubscribers() const { return this->subscribers; }

        /**
         * @brief           Subscribe all functions (subscribers) from other delegate to this delegate.
         * 
         * @param  other:   Other delegate reference.
         */
        void Combine(const DelegateBase &other)
        {
            for (size_t i = 0; i < other.subscribers.size(); i++)
            {
                this->subscribers.push_back(other.subscribers[i]);
                AttachParameters(other.parameters[i].parameters, std::index_sequence_for<Params...>());
            }
        }

        /**
         * @brief           Subscribes single function and saves single parameters pack.
         * @note   
         * @param  function:    Function to subscribe.
         * @param  params:      Parameters pack for the function to subscribe.
         * @retval None
         */
        void Subscribe(const FunctionType &function, Params... params)
        {
            this->subscribers.push_back(function);
            AttachParameters(std::tuple<Params...>(params...), std::index_sequence_for<Params...>());
        }

        /**
         * @brief           Subscribes multiple functions and saves single parameters pack.
         * @note   
         * @param  functions:   Multiple functions to subscribe.
         * @param  params:      Parameters pack for the functions to subscribe.
         * @retval None
         */
        void Subscribe(const std::initializer_list<FunctionType> &functions, Params... params)
        {
            for (auto &&d : functions)
            {
                this->subscribers.push_back(d);
                AttachParameters(std::tuple<Params...>(params...), std::index_sequence_for<Params...>());
            }
        }

        /**
         * @brief           Subscribes single function and save multiple parameters packs.
         * @note   
         * @param  function:    Function to subscribe.
         * @param  params:      Multiple parameter packs for the function to subscribe.
         * @retval None
         */
        void Subscribe(const FunctionType &function, std::vector<std::tuple<Params...>> params)
        {
            for (size_t i = 0; i < params.size(); i++)
            {
                this->subscribers.push_back(function);
                AttachParameters(params[i], std::index_sequence_for<Params...>());
            }
        }

        /**
         * @brief           Call all subscribed functions of this delegate that have parameters saved on subscription.
         */
        void Invoke()
        {
            for (size_t i = 0; i < parameters.size(); i++)
            {
                HelperInvoke(parameters[i].parameters, parameters[i].index,
                             std::index_sequence_for<Params...>());
            }
            return;
        }

        /**
         * @brief           Remove *count* functions from the back or front.
         * @note   
         * @param  count:       Count of functions to remove.
         * @param  fromBack:    If *true* - removing will be performed from back of the subscribers vector. From front otherwise.
         * @retval None
         */
        void Remove(int count = 1, bool fromBack = true)
        {
            int adjustedCount = count < subscribers.size() ? count : subscribers.size() - 1;

            if (!fromBack)
            {
                for (size_t i = 0; i < adjustedCount; ++i)
                {
                    DetachParameters(0);
                }
                subscribers.erase(subscribers.begin(), subscribers.begin() + count);
                return;
            }

            for (size_t i = 0; i < adjustedCount; ++i)
            {
                DetachParameters(parameters.size() - 1);
            }

            for (size_t i = 0; i < adjustedCount; i++)
            {
                subscribers.pop_back();
            }
        }

        /**
         * @brief  Is not implemented yet.
         * @note   
         * @param  subscriber: 
         */
        void Remove(const FunctionType &subscriber)
        {
            std::cout << "Removing " << &subscriber << std::endl;
            subscribers.erase(
                std::remove_if(
                    subscribers.begin(),
                    subscribers.end(),
                    [subscriber](const FunctionType &x) { return &x == &subscriber; }),
                subscribers.end());
            for (auto i : subscribers)
            {
                std::cout << "Address = " << &i << std::endl;
            }
        }

        /**
         * @brief           Remove all subscribers of this delegate appearing in the ***subscribers*** parameter.
         * @note            Is not implemented yet.
         * @param  subscribers: *std*::vector of functions that must be removed from the delegate.
         */
        void Remove(const std::vector<FunctionType> &subscribers)
        {
            for (auto &s : subscribers)
            {
                this->subscribers.erase(
                    std::remove_if(
                        this->subscribers.begin(),
                        this->subscribers.end(),
                        [s](const FunctionType &x) { return &s == &x; }),
                    this->subscribers.end());
            }
        }

        /**
         * @brief           Remove all subscribed functions from this delegate.
         * @returns *None*
         */
        void Clear()
        {
            this->subscribers.clear();
            this->parameters.clear();
        }

        /**
         * @brief           Subscribe function to this delegate.
         * 
         * @param  rhs:     Function to subscribe.
         * @returns         Reference to the delegate instance.
         */
        DelegateBase &operator+=(const FunctionType &rhs)
        {
            this->subscribers.push_back(rhs);
            return *this;
        }

        DelegateBase &operator+=(const std::initializer_list<FunctionType> &rhs)
        {
            for (auto x : rhs)
            {
                this->subscribers.push_back(x);
            }
            return *this;
        }

        /**
         * @brief           Unsubscribe choosen function from this delegate.
         * 
         * @param  rhs:     Function to unsubscribe from this delegate.
         * @returns         Reference to the delegate instance.
         */
        DelegateBase &operator-=(const FunctionType &rhs)
        {
            subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), rhs), subscribers.end());
            return *this;
        }

        DelegateBase &operator-=(const std::initializer_list<FunctionType> &rhs)
        {
            auto toRemove = [&](const FunctionType &func) -> bool {
                return std::find(rhs.begin(), rhs.end(), func) != rhs.end();
            };

            subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(), toRemove), subscribers.end());
            return *this;
        }

        DelegateBase &operator++()
        {
            if (subscribers.empty())
            {
                return *this;
            }

            FunctionType newDel = subscribers.front();

            if (parameters.empty())
            {
                subscribers.insert(subscribers.begin(), newDel);
                return *this;
            }

            subscribers.insert(subscribers.begin(), newDel);
            AttachParameters(parameters.front().parameters, std::index_sequence_for<Params...>());

            return *this;
        }

        DelegateBase &operator++(int)
        {
            if (subscribers.empty())
            {
                return *this;
            }

            FunctionType newDel = subscribers.back();

            if (parameters.empty())
            {
                subscribers.push_back(newDel);
                return *this;
            }

            subscribers.push_back(newDel);
            AttachParameters(parameters.back().parameters, std::index_sequence_for<Params...>());

            return *this;
        }

        DelegateBase &operator--()
        {
            if (subscribers.empty())
            {
                return *this;
            }

            if (parameters.empty())
            {
                subscribers.pop_back();
                return *this;
            }

            int index = parameters.back().index;
            parameters.pop_back();
            subscribers.erase(subscribers.begin() + index);

            return *this;
        }

        DelegateBase &operator--(int)
        {
            if (subscribers.empty())
            {
                return *this;
            }

            if (parameters.empty())
            {
                subscribers.pop_back();
                return *this;
            }

            int index = parameters.back().index;
            parameters.pop_back();
            subscribers.erase(subscribers.begin() + index);

            return *this;
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if count of subscribers of this delegate is less than other's.
         */
        bool operator<(const DelegateBase &rhs)
        {
            return subscribers.size() < rhs.subscribers.size();
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if count of subscribers of this delegate is less or equals to other's.
         */
        bool operator<=(const DelegateBase &rhs)
        {
            return subscribers.size() <= rhs.subscribers.size();
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if count of subscribers of this delegate is more than other's.
         */
        bool operator>(const DelegateBase &rhs)
        {
            return subscribers.size() > rhs.subscribers.size();
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if count of subscribers of this delegate is more or equals to other's.
         */
        bool operator>=(const DelegateBase &rhs)
        {
            return subscribers.size() >= rhs.subscribers.size();
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if subscribers of this delegate are equal to other's.
         */
        bool operator==(const DelegateBase &rhs)
        {
            return subscribers == rhs.subscribers;
        }

        /**
         * @brief           Signature of comparable delegates must match exactly.
         * @param  rhs:     Other delegate.
         * @returns         true if subscribers of this delegate are not-equal to other's.
         */
        bool operator!=(const DelegateBase &rhs)
        {
            return subscribers != rhs.subscribers;
        }

        /**
         * @brief           Transfer all subscribers of other delegate to this delegate. Will clear subscribers from other delegate.
         * @param  rhs:     Other delegate.
         * @returns         Pointer to this delegate.
         */
        DelegateBase &operator<<(DelegateBase &rhs)
        {
            if (this != &rhs)
            {
                this->Combine(rhs);
                rhs.Clear();
            }
            return *this;
        }

        /**
         * @brief           Transfer all subscribers of this delegate to other delegate. Will clear subscribers from this delegate.
         * @param  rhs:     Other delegate.
         * @returns         Pointer to this delegate.
         */
        DelegateBase &operator>>(DelegateBase &rhs)
        {
            if (this != &rhs)
            {
                rhs.Combine(*this);
                this->Clear();
            }
            return *this;
        }

        void DebugPrintParametersIndices()
        {
            for (auto &p : parameters)
            {
                std::cout << p.index << std::endl;
            }
        }

    protected:
        template <size_t... Indices>
        ReturnType HelperInvoke(const std::tuple<Params...> &tuple, int index, std::index_sequence<Indices...>)
        {
            return subscribers[index](std::get<Indices>(tuple)...);
        }

    private:
        void AttachParameters(Params... params)
        {
            this->parameters.push_back(FunctionParams<Params...>{subscribers.size() - 1, params...});
        }
        template <size_t... Indices>
        void AttachParameters(const std::tuple<Params...> &tuple, std::index_sequence<Indices...>)
        {
            this->parameters.push_back(FunctionParams<Params...>{subscribers.size() - 1, tuple});
        }
        void DetachParameters(const size_t index)
        {
            int erasedCount = 0;

            // Removing all parameters with specified index
            parameters.erase(
                std::remove_if(
                    parameters.begin(),
                    parameters.end(),
                    [index, &erasedCount](const FunctionParams<Params...> &x) {
                        if (x.index == index)
                        {
                            erasedCount++;
                            return true;
                        }
                        return false;
                    }),
                parameters.end());

            if (index > 0)
            {
                return;
            }

            // Correcting indices of saved parameters
            for (size_t i = 0; i < parameters.size(); ++i)
            {
                parameters[i].index -= erasedCount;
            }
        }
    };

    /**
     * @brief  Delegate is a class that encapsulates a function(s).
     * @note   
     * @tparam Params: Any number of arguments of any type.
     */
    template <typename... Params>
    class Delegate : public DelegateBase<void, Params...>
    {
    public:
        using Parent = DelegateBase<void, Params...>;
        using Parent::Clear;
        using Parent::Invoke;
        using Parent::subscribers;
        using typename Parent::FunctionType;

        /**
         * @brief           Invoke all subscribed functions.
         * 
         * @param  params:  Arguments of each subscribed function.
         */
        void operator()(Params... params)
        {
            for (auto &&i : subscribers)
            {
                i(params...);
            }
        }
    };

    /**
     * @brief               Delegate with any return type specified.
     * 
     * @tparam              ReturnType Return type of the Delegate.
     * @tparam              Params Any number of arguments of any type.
     */
    template <typename ReturnType, typename... Params>
    class RetDelegate : public DelegateBase<ReturnType, Params...>
    {
        static_assert(!std::is_void<ReturnType>::value, "RetDelegate can't have void return type!");

    public:
        using Parent = DelegateBase<ReturnType, Params...>;
        using Parent::Clear;
        using Parent::HelperInvoke;
        using Parent::parameters;
        using Parent::subscribers;
        using typename Parent::FunctionType;

        /**
         * @brief           Call all subscribed functions of this delegate that have parameters saved on subscription.
         * @note            
         * @returns         Sum of results of each function invocation.
         */
        ReturnType Invoke()
        {
            ReturnType result = ReturnType();
            for (size_t i = 0; i < parameters.size(); i++)
            {
                result += HelperInvoke(parameters[i].parameters, i, std::index_sequence_for<Params...>());
            }
            return result;
        }

        /**
         * @brief           Invoke all functions subscribed to this delegate.
         * 
         * @param  params:  Arguments of each subscribed function.
         * @returns         Sum of all subscribed functions results.
         */
        ReturnType operator()(Params... params)
        {
            ReturnType sum = ReturnType();
            for (auto &&i : subscribers)
            {
                sum += i(params...);
            }
            return sum;
        }
    };

    /**
     * @brief  Delegate that holds the subscribed member functions.
     * @note   
     * @tparam ReturnType   Return type of the Delegate.
     * @tparam ObjType      Type of the member function owner class.
     * @tparam Params       Any number of arguments of any type.
     */
    template <typename ReturnType, class ObjType, typename... Params>
    class MemberDelegateBase
    {
        template <typename... T>
        struct MemberFunctionParams
        {
            size_t index = -1;
            ObjType *object;
            std::tuple<T...> parameters;
        };

    public:
        typedef ReturnType (ObjType::*MemberFunctionType)(Params...);

    protected:
        /**
         * @brief           **std::vector** of methods that are subscribed to this delegate.
         */
        std::vector<MemberFunctionType> subscribers;

        /**
         * @brief           Vector of parameters saved when each method is subscribed to this delegate.
         */
        std::vector<MemberFunctionParams<Params...>> parameters;

        MemberDelegateBase() = default;

    public:

        /**
         * @brief           Subscribe single method for a choosen object with the specified parameters.
         * @note   
         * @param  obj:     Pointer to an object that holds the specified method that will be subscribed.
         * @param  delegate: 
         * @param  params: 
         * @retval None
         */
        void Subscribe(ObjType *obj, const MemberFunctionType &method, Params... params)
        {
            subscribers.push_back(method);
            AttachParameters(obj, std::tuple<Params...>(params...), std::index_sequence_for<Params...>());
        }

        void Clear()
        {
            subscribers->clear();
            parameters->clear();
        }

        /**
         * @brief           Subscribe method to this delegate.
         * 
         * @param  rhs:     Method to subscribe.
         * @returns         Reference to the delegate instance.
         */
        MemberDelegateBase &operator+=(const MemberFunctionType &rhs)
        {
            subscribers.push_back(rhs);
            return *this;
        }

        /**
         * @brief           Subscribe multiple methods to this delegate.
         * @note   
         * @param  rhs:     Methods to subscribe. 
         * @retval          Reference to the delegate instance.
         */
        MemberDelegateBase &operator+=(const std::initializer_list<MemberFunctionType> &rhs)
        {
            for (auto x : rhs)
            {
                this->subscribers.push_back(x);
            }
            return *this;
        }

        /**
         * @brief           Unsubscribe choosen method from this delegate.
         * 
         * @param  rhs:     Method to unsubscribe from this delegate.
         * @returns         Reference to the delegate instance.
         */
        MemberDelegateBase &operator-=(const MemberFunctionType &rhs)
        {
            subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), rhs), subscribers.end());
            return *this;
        }

        /**
         * @brief           Unsubscribe multiple methods from this delegate.
         * @note   
         * @param  rhs:     Methods to unsubscribe from this delegate. 
         * @retval          Reference to the delegate instance.
         */
        MemberDelegateBase &operator-=(const std::initializer_list<MemberFunctionType> &rhs)
        {
            auto toRemove = [&](const MemberFunctionType &delegate) -> bool {
                return std::find(rhs.begin(), rhs.end(), delegate) != rhs.end();
            };

            subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(), toRemove), subscribers.end());
            return *this;
        }

    private:
        void AttachParameters(ObjType *obj, Params... params)
        {
            this->parameters.push_back(MemberFunctionParams<Params...>{subscribers.size() - 1, obj, params...});
        }
        template <size_t... Indices>
        void AttachParameters(ObjType *obj, const std::tuple<Params...> &tuple, std::index_sequence<Indices...>)
        {
            this->parameters.push_back(MemberFunctionParams<Params...>{subscribers.size() - 1, obj, tuple});
        }
    };

    template <class ObjType, typename... Params>
    class MemberDelegate : public MemberDelegateBase<void, ObjType, Params...>
    {
    public:
        using Parent = MemberDelegateBase<void, ObjType, Params...>;
        using Parent::parameters;
        using Parent::subscribers;
        using typename Parent::MemberFunctionType;

        /**
         * @brief           Call all subscribed methods of this delegate that have parameters saved on subscription.
         */
        void Invoke()
        {
            for (size_t i = 0; i < parameters.size(); i++)
            {
                HelperMemberInvoke(parameters[i].object, parameters[i].parameters, i, std::index_sequence_for<Params...>());
            }
            return;
        }

        /**
         * @brief           Calls subscribed methods with the specified parameters.
         * @param  obj:     Pointer to an object that will call *all* subscribed methods of this delegate.
         * @param  params:  Method parameters pack.
         * @retval None
         */
        void operator()(ObjType *obj, Params... params)
        {
            for (auto &&i : subscribers)
            {
                (obj->*i)(params...);
            }
        }

    private:
        template <size_t... Indices>
        void HelperMemberInvoke(ObjType *obj, const std::tuple<Params...> &tuple, int index, std::index_sequence<Indices...>)
        {
            (obj->*subscribers[index])(std::get<Indices>(tuple)...);
        }
    };

    template <typename ReturnType, class ObjType, typename... Params>
    class RetMemberDelegate : public MemberDelegateBase<ReturnType, ObjType, Params...>
    {
        static_assert(!std::is_void<ReturnType>::value, "RetMemberDelegate can't have void return type!");

    public:
        using Parent = MemberDelegateBase<ReturnType, ObjType, Params...>;
        using Parent::parameters;
        using Parent::subscribers;
        using typename Parent::MemberFunctionType;

        /**
         * @brief           Call all subscribed methods of this delegate that have parameters saved on subscription.
         */
        ReturnType Invoke()
        {
            ReturnType result = ReturnType();
            for (size_t i = 0; i < parameters.size(); i++)
            {
                result += HelperMemberInvoke(parameters[i].object, parameters[i].parameters, i, std::index_sequence_for<Params...>());
            }
            return result;
        }

        /**
         * @brief           Calls subscribed methods with the specified parameters.
         * @param  obj:     Pointer to an object that will call *all* subscribed methods of this delegate.
         * @param  params:  Method parameters pack.
         * @retval None
         */
        ReturnType operator()(ObjType *obj, Params... params)
        {
            ReturnType result = ReturnType();
            for (auto &&i : subscribers)
            {
                result += (obj->*i)(params...);
            }
            return result;
        }

    private:
        template <size_t... Indices>
        ReturnType HelperMemberInvoke(ObjType *obj, const std::tuple<Params...> &tuple, int index, std::index_sequence<Indices...>)
        {
            return (obj->*subscribers[index])(std::get<Indices>(tuple)...);
        }
    };

    // template <typename... Params>
    // class MasterDelegate : public Delegate<Params...>
    // {
    //     template<typename... Args>
    //     struct MasterArgs
    //     {
    //         std::tuple<Args...> args;
    //     };

    //     //std::vector<std::any> masterArgs;

    // public:
    //     using typename DelegateBase<void, Params...>::DelegateType;
    //     using DelegateBase<void, Params...>::subscribers;
    //     using DelegateBase<void, Params...>::Clear;
    //     using DelegateBase<void, Params...>::Invoke;

    //     template<typename T, typename Args>
    //     T InvokeAll(T type, Args... args)
    //     {
    //         return T();
    //     }

    //     template<typename Returns, typename... Args>
    //     Returns AddUnique(Args... args)
    //     {
    //         return Returns();
    //     }
    // };

} // namespace dw