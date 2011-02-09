//-----------------------------------------------------------
//    wrap_function.h
//-----------------------------------------------------------
#ifndef SYKES_WRAP_FUNCTION_H
#define SYKES_WRAP_FUNCTION_H

#include <tuple>
#include <utility>
#include <type_traits>

namespace sykes{
    //-----------------------------------------------------------
    //    wrap_function
    //-----------------------------------------------------------
    template<typename F, typename Forig, typename ... CastFuncs>
    class wrap_function;
    
    //-----------------------------------------------------------
    //    nocast
    //-----------------------------------------------------------
    struct nocast{};
    
    //-----------------------------------------------------------
    //    make_wrap_function
    //-----------------------------------------------------------
    template<typename Fcall_wf, typename Forig, typename ... CastFuncs>
    wrap_function<Fcall_wf, Forig, CastFuncs...>
    make_wrap_function(Forig func, CastFuncs ... castf)
    {
        return wrap_function<Fcall_wf, Forig, CastFuncs...>(func, castf...);
    }
    
    template<typename Fcall_wf, typename Forig, typename ... CastFuncs>
    wrap_function<Fcall_wf, Forig, CastFuncs...>
    make_wrap_function(Forig func, std::tuple<CastFuncs ...> const& castf)
    {
        return wrap_function<Fcall_wf, Forig, CastFuncs...>(func, castf);
    }
    
    namespace detail{
        template<bool NoCast, std::size_t M, typename Forig, typename Fcall, typename Cast>
        struct call_wf;
        
        template<std::size_t N, typename Cast>
        struct check_nocast;
    }

    //-----------------------------------------------------------
    //    wrap_function
    //-----------------------------------------------------------
    template<typename R, typename ... Args, typename Forig, typename ... CastFuncs>
    class wrap_function<R(Args...), Forig, CastFuncs...>
    {
        static_assert(sizeof...(Args) == sizeof...(CastFuncs), "wrap_function");
    private:
        static std::size_t const argnum = sizeof...(Args);
        Forig m_forig;
        std::tuple<CastFuncs...> m_cast_funcs;
        
    public:
        wrap_function(Forig f, CastFuncs ... castf)
            : m_forig(f), m_cast_funcs(castf...)
        {}
        
        wrap_function(Forig f, std::tuple<CastFuncs...> const& castf)
            : m_forig(f), m_cast_funcs(castf)
        {
            static_assert(sizeof...(CastFuncs) != 0, "sykes::wrap_function::wrap_function()");
        }
        
        R operator()(Args ... args)
        {
            std::tuple<Args...> pack = std::tuple<Args...>(args...);
            return detail::call_wf<detail::check_nocast<argnum - 1, std::tuple<CastFuncs...>>::value,
                argnum, Forig, R(Args...), std::tuple<CastFuncs...>>::execute(
                    m_forig, pack, m_cast_funcs);
        }
    };

    //-----------------------------------------------------------
    //    detail
    //-----------------------------------------------------------
    namespace detail{
        //-----------------------------------------------------------
        //    check_nocast
        //-----------------------------------------------------------
        template<std::size_t N, typename Cast>
        struct check_nocast
        {
            static bool const value = std::is_same<
                nocast,
                typename std::remove_cv<
                    typename std::remove_reference<
                        typename std::tuple_element<
                            N, Cast
                        >::type
                    >::type
                >::type
            >::value;
        };
        
        //-----------------------------------------------------------
        //    call_wf
        //-----------------------------------------------------------
        template<typename F, typename R1, typename ... A1, typename CastF>
        struct call_wf<false, 1, F, R1(A1...), CastF>
        {
            template<typename ... Casted>
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                return f(std::get<0>(castf)(std::get<0>(tup)), a...);
            }
            
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                return f(std::get<0>(castf)(std::get<0>(tup)));
            }
        };
        
        template<typename F, typename R1, typename ... A1, typename CastF>
        struct call_wf<true, 1, F, R1(A1...), CastF>
        {
            template<typename ... Casted>
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                return f(std::get<0>(tup), a...);
            }
            
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                return f(std::get<0>(tup));
            }
        };
        
        template<std::size_t M, typename F, typename R1, typename ... A1, typename CastF>
        struct call_wf<false, M, F, R1(A1...), CastF>
        {
            template<typename ... Casted>
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                return call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, R1(A1...), CastF>::execute(f, tup, castf,
                    std::get<M - 1>(castf)(std::get<M - 1>(tup)), a...);
            }
            
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                return call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, R1(A1...), CastF>::execute(f, tup, castf, std::get<M - 1>(castf)(std::get<M - 1>(tup)));
            }
        };
        
        template<std::size_t M, typename F, typename R1, typename ... A1, typename CastF>
        struct call_wf<true, M, F, R1(A1...), CastF>
        {
            template<typename ... Casted>
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                return call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, R1(A1...), CastF>::execute(f, tup, castf,
                    std::get<M - 1>(tup), a...);
            }
            
            static R1 execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                return call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, R1(A1...), CastF>::execute(
                    f, tup, castf, std::get<M - 1>(tup));
            }
        };
        
        //-----------------------------------------------------------
        //    specialization for void
        //-----------------------------------------------------------
        template<typename F, typename ... A1, typename CastF>
        struct call_wf<false, 1, F, void(A1...), CastF>
        {
            template<typename ... Casted>
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                f(std::get<0>(castf)(std::get<0>(tup)), a...);
            }
            
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                f(std::get<0>(castf)(std::get<0>(tup)));
            }
        };
        
        template<typename F, typename ... A1, typename CastF>
        struct call_wf<true, 1, F, void(A1...), CastF>
        {
            template<typename ... Casted>
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                f(std::get<0>(tup), a...);
            }
            
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                f(std::get<0>(tup));
            }
        };
        
        template<std::size_t M, typename F, typename ... A1, typename CastF>
        struct call_wf<false, M, F, void(A1...), CastF>
        {
            template<typename ... Casted>
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, void(A1...), CastF>::execute(f, tup, castf,
                    std::get<M - 1>(castf)(std::get<M - 1>(tup)), a...);
            }
            
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, void(A1...), CastF>::execute(f, tup, castf, std::get<M - 1>(castf)(std::get<M - 1>(tup)));
            }
        };
        
        template<std::size_t M, typename F, typename ... A1, typename CastF>
        struct call_wf<true, M, F, void(A1...), CastF>
        {
            template<typename ... Casted>
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf, Casted&& ... a)
            {
                call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, void(A1...), CastF>::execute(f, tup, castf,
                    std::get<M - 1>(tup), a...);
            }
            
            static void execute(F f, std::tuple<A1...>& tup, CastF& castf)
            {
                call_wf<check_nocast<M - 2, CastF>::value, M - 1, F, void(A1...), CastF>::execute(
                    f, tup, castf, std::get<M - 1>(tup));
            }
        };
    }
}//----
#endif

