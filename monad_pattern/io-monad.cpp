
#include <memory>
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <iterator>

struct sequence_tag {};
struct pointer_tag {};

template< class X >
X category( ... );

template< class S >
auto category( const S& s ) -> decltype( std::begin(s), sequence_tag() );

template< class Ptr >
auto category( const Ptr& p ) -> decltype( *p, p==nullptr, pointer_tag() );

template< class T > struct Category {
    using type = decltype( category<T>(std::declval<T>()) );
};

template< class R, class ... X > struct Category< R(&)(X...) > {
    using type = R(&)(X...);
};

template< class T >
using Cat = typename Category< typename std::decay<T>::type >::type;

template<class...> struct PartialApplication;

template< class F, class X >
struct PartialApplication< F, X >
{
    F f;
    X x;

    template< class _F, class _X >
    constexpr PartialApplication( _F&& f, _X&& x )
        : f(std::forward<_F>(f)), x(std::forward<_X>(x))
    {
    }

    /* 
     * The return type of F only gets deduced based on the number of xuments
     * supplied. PartialApplication otherwise has no idea whether f takes 1 or 10 xs.
     */
    template< class ... Xs >
    constexpr auto operator() ( Xs&& ...xs )
        -> decltype( f(x,std::declval<Xs>()...) )
    {
        return f( x, std::forward<Xs>(xs)... );
    }
};

/* Recursive, variadic version. */
template< class F, class X1, class ...Xs >
struct PartialApplication< F, X1, Xs... > 
    : public PartialApplication< PartialApplication<F,X1>, Xs... >
{
    template< class _F, class _X1, class ..._Xs >
    constexpr PartialApplication( _F&& f, _X1&& x1, _Xs&& ...xs )
        : PartialApplication< PartialApplication<F,X1>, Xs... > (
            PartialApplication<F,X1>( std::forward<_F>(f), std::forward<_X1>(x1) ),
            std::forward<_Xs>(xs)...
        )
    {
    }
};

/* 
 * Some languages implement partial application through closures, which hold
 * references to the function's arguments. But they also often use reference
 * counting. We must consider the scope of the variables we want to apply. If
 * we apply references and then return the applied function, its references
 * will dangle.
 *
 * See: 
 * upward funarg problem http://en.wikipedia.org/wiki/Upward_funarg_problem
 */

/*
 * closure http://en.wikipedia.org/wiki/Closure_%28computer_science%29
 * Here, closure forwards the arguments, which may be references or rvalues--it
 * does not matter. A regular closure works for passing functions down.
 */
template< class F, class ...X >
constexpr PartialApplication<F,X...> closure( F&& f, X&& ...x ) {
    return PartialApplication<F,X...>( std::forward<F>(f), std::forward<X>(x)... );
}

/*
 * Thinking as closures as open (having references to variables outside of
 * itself), let's refer to a closet as closed. It contains a function and its
 * arguments (or environment).
 */
template< class F, class ...X >
constexpr PartialApplication<F,X...> closet( F f, X ...x ) {
    return PartialApplication<F,X...>( std::move(f), std::move(x)... );
}

template< class F, class ...G >
struct Composition;

template< class F, class G >
struct Composition<F,G>
{
    F f; G g;

    template< class _F, class _G >
    constexpr Composition( _F&& f, _G&& g ) 
        : f(std::forward<_F>(f)), g(std::forward<_G>(g)) { }

    template< class X, class ...Y >
    constexpr decltype( f(g(std::declval<X>()), std::declval<Y>()...) )
    operator() ( X&& x, Y&& ...y ) {
        return f( g( std::forward<X>(x) ), std::forward<Y>(y)... );
    }

    constexpr decltype( f(g()) ) operator () () {
        return f(g());
    }
};

template< class F, class G, class ...H >
struct Composition<F,G,H...> : Composition<F,Composition<G,H...>>
{
    typedef Composition<G,H...> Comp;

    template< class _F, class _G, class ..._H >
    constexpr Composition( _F&& f, _G&& g, _H&& ...h )
        : Composition<_F,Composition<_G,_H...>> ( 
            std::forward<_F>(f), 
            Comp( std::forward<_G>(g), std::forward<_H>(h)... )
        )
    {
    }
};

template< class F, class ...G >
constexpr Composition<F,G...> compose( F f, G ...g ) {
    return Composition<F,G...>( std::move(f), std::move(g)... );
}

template< class... > struct Functor;

template< class F, class FX, class Fun=Functor< Cat<FX> > >
constexpr auto fmap( F&& f, FX&& fx ) 
    -> decltype( Fun::fmap( std::declval<F>(), std::declval<FX>() ) )
{
    return Fun::fmap( std::forward<F>(f), std::forward<FX>(fx) );
}

// General case: compose
template< class Function > struct Functor<Function> {
    template< class F, class G, class C = Composition<F,G> >
    static constexpr C fmap( F f, G g ) {
        C( std::move(f), std::move(g) );
    }
};

template<> struct Functor< sequence_tag > {
    template< class F, template<class...>class S, class X,
              class R = typename std::result_of<F(X)>::type >
    static S<R> fmap( F&& f, const S<X>& s ) {
        S<R> r;
        r.reserve( s.size() );
        std::transform( std::begin(s), std::end(s), 
                        std::back_inserter(r), 
                        std::forward<F>(f) );
        return r;
    }
};

template<> struct Functor< pointer_tag > {
    template< class F, template<class...>class Ptr, class X,
              class R = typename std::result_of<F(X)>::type >
    static Ptr<R> fmap( F&& f, const Ptr<X>& p ) 
    {
        return p != nullptr 
            ? Ptr<R>( new R( std::forward<F>(f)(*p) ) )
            : nullptr;
    }
};

template< class ... > struct Monad;

template< class F, class M, class ...N, class Mo=Monad<Cat<M>> >
constexpr auto mbind( F&& f, M&& m, N&& ...n ) 
    -> decltype( Mo::mbind(std::declval<F>(),
                           std::declval<M>(),std::declval<N>()...) )
{
    return Mo::mbind( std::forward<F>(f), 
                      std::forward<M>(m), std::forward<N>(n)... );
}

template< class F, class M, class ...N, class Mo=Monad<Cat<M>> >
constexpr auto mdo( F&& f, M&& m ) 
    -> decltype( Mo::mdo(std::declval<F>(), std::declval<M>()) )
{
    return Mo::mdo( std::forward<F>(f), std::forward<M>(m) );
}

// The first template argument must be explicit!
template< class M, class X, class ...Y, class Mo = Monad<Cat<M>> >
constexpr auto mreturn( X&& x, Y&& ...y ) 
    -> decltype( Mo::template mreturn<M>( std::declval<X>(),
                                          std::declval<Y>()... ) )
{
    return Mo::template mreturn<M>( std::forward<X>(x), 
                                    std::forward<Y>(y)... );
}

template< template<class...>class M, class X, class ...Y,
    class _M = M< typename std::decay<X>::type >,
    class Mo = Monad<Cat<_M>> >
constexpr auto mreturn( X&& x, Y&& ...y ) 
    -> decltype( Mo::template mreturn<_M>( std::declval<X>(),
                                           std::declval<Y>()... ) )
{
    return Mo::template mreturn<_M>( std::forward<X>(x),
                                     std::forward<Y>(y)... );
}

// Also has explicit template argument.
template< class M, class Mo = Monad<Cat<M>> >
auto mfail() -> decltype( Mo::template mfail<M>() ) {
    return Mo::template mfail<M>();
}

template< class M, class F >
constexpr auto operator >>= ( M&& m, F&& f ) 
    -> decltype( mbind(std::declval<F>(),std::declval<M>()) )
{
    return mbind( std::forward<F>(f), std::forward<M>(m) );
}

template< class M, class F >
constexpr auto operator >> ( M&& m, F&& f ) 
    -> decltype( mdo(std::declval<M>(),std::declval<F>()) )
{
    return mdo( std::forward<M>(m), std::forward<F>(f) );
}

template< class F, class M >
constexpr auto operator ^ ( F&& f, M&& m ) 
    -> decltype( fmap(std::declval<F>(),std::declval<M>()) )
{
    return fmap( std::forward<F>(f), std::forward<M>(m) );
}

template< > struct Monad< pointer_tag > {

    template< class P >
    using mvalue = typename P::element_type;

    template< class F, template<class...>class Ptr, class X,
              class R = typename std::result_of<F(X)>::type >
    static R mbind( F&& f, const Ptr<X>& p ) {
        return p ? std::forward<F>(f)( *p ) : nullptr;
    }

    template< class F, template<class...>class Ptr, 
              class X, class Y,
              class R = typename std::result_of<F(X,Y)>::type >
    static R mbind( F&& f, const Ptr<X>& p, const Ptr<Y>& q ) {
        return p and q ? std::forward<F>(f)( *p, *q ) : nullptr;
    }

    template< template< class... > class M, class X, class Y >
    static M<Y> mdo( const M<X>& mx, const M<Y>& my ) {
        return mx ? (my ? mreturn<M<Y>>(*my) : nullptr) 
            : nullptr;
    }

    template< class M, class X >
    static M mreturn( X&& x ) {
        using Y = typename M::element_type;
        return M( new Y(std::forward<X>(x)) );
    }

    template< class M >
    static M mfail() { return nullptr; }
}; 

template< > struct Monad< sequence_tag > {

    template< class S >
    using mvalue = typename S::value_type;

    template< class F, template<class...>class S, class X,
        class R = typename std::result_of<F(X)>::type >
    static R mbind( F&& f, const S<X>& xs ) {
        R r;
        for( const X& x : xs ) {
            auto ys = std::forward<F>(f)( x );
            std::move( std::begin(ys), std::end(ys), std::back_inserter(r) );
        }
        return r;
    }

    template< class F, template<class...>class S, 
              class X, class Y,
              class R = typename std::result_of<F(X,Y)>::type >
    static R mbind( F&& f, const S<X>& xs, const S<Y>& ys ) {
        R r;
        for( const X& x : xs ) {
            for( const Y& y : ys ) {
                auto zs = std::forward<F>(f)( x, y );
                std::move( std::begin(zs), std::end(zs), 
                           std::back_inserter(r) );
            }
        }
        return r;
    }

    template< template< class... > class S, class X, class Y >
    static S<Y> mdo( const S<X>& mx, const S<Y>& my ) {
        // Note: This is not a strictly correct definition. 
        // It should return my concatenated to itself for every element of mx.
        return mx.size() ? my : S<Y>{};
    }

    template< class S, class X >
    static S mreturn( X&& x ) {
        return S{ std::forward<X>(x) }; // Construct an S of one element.
    }

    template< class S >
    static S mfail() { return S{}; }
};


template< class X > struct Identity {
    using value_type = X;
    using reference = value_type&;
    using const_reference = const value_type&;
    value_type x;

    template< class Y >
    Identity( Y&& y ) : x( std::forward<Y>(y) ) { }

    constexpr value_type operator () () { return x; }
};

template< class X, class D = typename std::decay<X>::type > 
Identity<D> identity( X&& x ) {
    return Identity<D>( std::forward<X>(x) );
}

template< class F > struct IO {
    using function = F;
    F f;

    constexpr IO( function f ) : f(std::move(f)) { }

    constexpr decltype(f()) operator () () {
        return f();
    }
};

template< class F, class _F = typename std::decay<F>::type >
constexpr IO<_F> io( F&& f ) {
    return std::forward<F>(f);
}

template< class F, class X, class ...Y >
constexpr auto io( F f, X x, Y ...y )  -> PartialApplication<F,X,Y...>
{
    return closet( std::move(f), std::move(x), std::move(y)... );
}

template< class T, class R >
using EVoid = typename std::enable_if< std::is_void<T>::value, R >::type;
template< class T, class R >
using XVoid = typename std::enable_if< !std::is_void<T>::value, R >::type;

template< class F, class G, class R = typename std::result_of<F()>::type >
constexpr auto incidentallyDo( F&& f, G&& g ) 
    -> XVoid <
        R,
        decltype( std::declval<G>()( std::declval<F>()() ) )
    >
{
    return std::forward<G>(g)( std::forward<F>(f)() );
}

template< class F, class G, class R = typename std::result_of<F()>::type >
auto incidentallyDo( F&& f, G&& g ) 
    -> EVoid <
        R,
        decltype( std::declval<G>()() )
    >
{
    std::forward<F>(f)();
    return std::forward<G>(g)();
}

template< class F, class G > struct Incidence {
    F a;
    G b;

    constexpr Incidence( F a, G b )
        : a(std::move(a)), b(std::move(b))
    {
    }

    // The Intermediate type.
    using I = typename std::result_of<F()>::type;

    using R = decltype( incidentallyDo(a,b) );

    constexpr R operator () () {
        return incidentallyDo( a, b );
    }
};

template< class F, class G, class I = Incidence<F,G> >
constexpr I incidence( F f, G g ) {
    return I( std::move(f), std::move(g) );
}

template< class F > struct DoubleCall {
    F f;

    using F2 = typename std::result_of<F()>::type;
    using result_type = typename std::result_of<F2()>::type;

    constexpr result_type operator () () {
        return f()();
    }
};

template< class F > 
constexpr DoubleCall<F> doubleCall( F f ) {
    return { std::move(f) };
}

template< class _ > struct Functor< IO<_> > {
    template< class F, class G, class I = Incidence<F,G> >
    constexpr static IO<I> fmap( F f, IO<G> r ) {
        return I( std::move(f), std::move(r.f) );
    }
};

template< class _ > struct Monad< IO<_> > {

    template< class F >
    using mvalue = typename std::result_of<F()>::type;

    template< class F, class G, 
        class R = typename std::result_of<G()>::type >
    static constexpr auto mbind( F f, IO<G> m ) 
        -> IO< DoubleCall< Incidence<G,F> > >
    {
        return doubleCall (
            incidence( std::move(m.f), std::move(f) )
        );
    }

    template< class F, class G >
    static constexpr IO<Incidence<F,G>> mdo( IO<F> f, IO<G> g ) {
        return incidence( std::move(f.f), std::move(g.f) );
    }

    template< class __, class X, class D = typename std::decay<X>::type >
    static constexpr IO<Identity<D>> mreturn( X&& x ) {
        return Identity<D>( std::forward<X>(x) );
    }

    template< class _IO >
    static _IO mfail() {
        return _IO( []{ } );
    }
};

template< class T >
struct ReadT {
    T operator () () const {
        T x;
        std::cin >> x;
        return x;
    }
};

template< class T >
constexpr IO<ReadT<T>> readT() { 
    return ReadT<T>();
}

static void printStr( const std::string& s ) {
    std::cout << s;
}

static void printCStr( const char* const s ) {
    std::cout << s;
}

constexpr struct Print {
    template< class X >
    void operator () ( const X& x ) const {
        std::cout << x;
    }
} print{};

template< class X >
static std::string show( const X& x ) {
    static std::ostringstream oss;
    oss.str( "" );
    oss << x;
    return oss.str();
}

static std::string show( std::string str ) {
    return str;
}

static constexpr const char* show( const char* str ) {
    return str;
}

template< class X, class Y, class ...Z >
static std::string show( const X& x, const Y& y, const Z& ...z ) 
{
    return show(x) + show(y,z...);
}

constexpr struct Echo {
    using F = PartialApplication< Print, std::string >;
    using result_type = IO<F>;

    template< class ...X >
    result_type operator () ( const X& ...x ) const {
        // We don't know if x will still be around when the IO executes, so
        // convert to a string right away!
        return closet( print, show(x...) );
    }

    using G = PartialApplication< Print, const char* >;

    constexpr IO<G> operator () ( const char* str ) {
        return closet( print, str );
    }
} echo{};

auto newline = io( []{ std::cout << std::endl; } );

template< class X, class Y, class Z >
auto equation( const std::string& op, 
               const X& x, const Y& y, const Z& z ) 
    -> decltype( echo(op) >> newline )
{
    return echo( x, op, y, " = ", z ) >> newline;
}

template< class M > struct _AddM {
    constexpr auto operator () ( int x, int y ) -> decltype( mreturn<M>(1) ) 
    {
        return mreturn<M>( x + y );
    }
};

constexpr struct BindCloset {
    template< class F, class X, class M >
    constexpr auto operator () ( F&& f, M&& m, X&& x ) 
        -> decltype( std::declval<M>() >>= 
                     closet(std::declval<F>(),std::declval<X>()) )
    {
        return std::forward<M>(m) >>= 
            closet( std::forward<F>(f), std::forward<X>(x) );
    }
} bindCloset{};

template< class M >
constexpr auto addM( const M& a, const M& b ) 
    -> decltype (
        a >>= closure( bindCloset, _AddM<M>(), b )
               
    )
{
    return a >>= closure( bindCloset, _AddM<M>(), b );
}

template< class M >
M addM2( const M& a, const M& b ) {
    return mbind (
        [&]( int x ) {
            return fmap (
                [=]( int y ) { return x + y; },
                b
            );
        },
        a
    );
}

template< class M >
using MonadValue = typename Monad<Cat<M>>::template mvalue<M>;

template< class F, template<class...>class M, class X, class Y >
auto liftM( const F& f, const M<X>& a, const M<Y>& b ) 
    -> M< typename std::result_of<F(X,Y)>::type >
{
    return a >>= [&]( const X& x ) {
        return b >>= [&]( const Y& y ) { 
            return mreturn<M>( f(x,y) );
        };
    };
};

/*
 * guard<M>(b) = (return True) or mfail().
 *
 * guard prematurely halts an execution based on some bool, b. Note that:
 *      p >> q = q
 *      mfail() >> p = mfail()
 *      nullptr >> p = nullptr -- where p is a unique_ptr.
 *      {} >> v = {} -- where v is a vector.
 */
template< template< class... > class M >
constexpr M<bool> guard( bool b ) {
    return b ? mreturn<M>(b) : mfail<M<bool>>();
}

/*
 * The above version of guard creates a junk Monad. This may be costly.
 * This version is an optimal shorthand for guard(b) >> m.
 */
template< template< class... > class M, class F,
          class R = typename std::result_of<F()>::type >
constexpr M<R> guard( bool b, F&& f ) {
    return b ? mreturn<M>( std::forward<F>(f)() ) : mfail<M<R>>();
}

template< template< class... > class M, class X >
M< std::pair<X,X> > uniquePairs( const M<X>& m ) {
    return mbind (
        []( int x, int y ) -> M< std::pair<X,X> > { 
            // This is a very Haskell-like use of guard.
            return guard<M>( x != y ) >> mreturn<M>( std::make_pair(x,y) );
        }, m, m
    );
}

/* alias for mreturn<unique_ptr> */
template< class X >
auto Just( X&& x ) -> decltype( mreturn<std::unique_ptr>(std::declval<X>()) ) {
    return mreturn<std::unique_ptr>( std::forward<X>(x) );
}

#include <cmath>
// Safe square root.
std::unique_ptr<float> sqrt( float x ) {
    // The more optimized C++-guard.
    return guard<std::unique_ptr>( x >= 0, [x]{ return std::sqrt(x); } );

    // Equivalently,
    return x >= 0 ? Just( std::sqrt(x) ) : nullptr;
}

// Safe quadratic root.
std::unique_ptr<std::pair<float,float>> qroot( float a, float b, float c ) {
    return fmap (
        [=]( float r /*root*/ ) {
            return std::make_pair( (-b + r)/(2*a), (-b - r)/(2*a) );
        }, 
        sqrt( b*b - 4*a*c )
    );
}

template< class X, class Y >
std::ostream& operator << ( std::ostream& os, const std::pair<X,Y>& p ) {
    os << '(' << p.first << ',' << p.second << ')';
    return os;
}

template< class X >
std::ostream& operator << ( std::ostream& os, const std::unique_ptr<X>& p ) {
    if( p ) 
        os << "Just " << *p;
    else
        os << "Nothing";
    return os;
}

template< class X >
std::ostream& operator << ( std::ostream& os, const std::vector<X>& v ) {
    os << '[';
    
    for( auto it=std::begin(v); it != std::end(v); it++ ) {
        os << *it;
        if( std::next(it) != std::end(v) )
            os << ',';
    }

    os << ']';
    return os;
}

int main() {
    std::unique_ptr<int> p( new int(5) );
    auto f = []( int x ) { return Just(-x); };
    std::unique_ptr<int> q = mbind( f, p );

    std::vector<int> v={1,2,3}, w={3,4};

    auto readInt = readT<int>();

    auto program = 
        equation( " + ", p, q, addM(p,q) ) >>
        equation( " + ", v, w, addM(v,w) )

        >> echo( "Unique pairs of [1,2,3]:\n\t" ) 
        >> echo( uniquePairs(v) ) >> newline 

        >> echo("Unique pairs of Just 5:\n\t")
        >> echo( uniquePairs(p) ) >> newline 

        >> echo( "Please enter two numbers, x and y: " ) 
        >> (
            addM( readInt, readInt ) >>= []( int x ) { 
                return echo( "x+y = ", x ) >> newline; 
            }
        ) 

        >> echo("The quadratic root of (1,3,-4) = ") 
            >> echo( qroot(1,3,-4) ) >> newline 
        >> echo("The quadratic root of (1,0,4) = ") 
            >> echo( qroot(1,0,4) ) >> newline;

    program();

}

