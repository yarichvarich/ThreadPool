#include <ThreadPool.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>

using namespace std::chrono_literals;
using ldouble = long double;

ThreadPool g_pool{4u};

class ScopedProfiler
{
    public:

        ScopedProfiler();

       ~ScopedProfiler();

    private:

        std::chrono::time_point<std::chrono::high_resolution_clock> m_creationTime;
};

class MandelbrotRenderer
{
    public:

        static void init() { s_instance = new MandelbrotRenderer{}; }

        static void deinit() { delete s_instance; }

        static void setLeftEdge(ldouble leftEdge);

        static void setRightEdge(ldouble rightEdge);

        static void setTopEdge(ldouble topEdge);

        static void setBottomEdge(ldouble bottomEdge);

        static ldouble getLeftEdge();

        static ldouble getRightEdge();

        static ldouble getTopEdge();

        static ldouble getBottomEdge();

        inline static const ldouble map(ldouble val, ldouble a0, ldouble b0, ldouble a1, ldouble b1);

        static const sf::Color calculateMandelbrotColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        enum class BatchingStrategy
        {
            DISABLE_BATCHING,
            ENABLE_BATCHING
        };

        enum class ThreadingStrategy
        {
            DISABLE_THREADPOOL,
            ENABLE_THREADPOOL
        };

        template<BatchingStrategy batchingStrategy = BatchingStrategy::DISABLE_BATCHING,
                 ThreadingStrategy threadingStrategy = ThreadingStrategy::DISABLE_THREADPOOL>
        static void render(sf::Image& inoutImage);

    private:

        MandelbrotRenderer() = default;

        inline static MandelbrotRenderer* s_instance = nullptr;
        
        uint32_t m_iterations = 1000u;

        ldouble m_left = -2.;
        ldouble m_right = 2.;
        ldouble m_top = 2.;
        ldouble m_bottom = -2.;
};

class MandelbrotNavigator
{
    public:

        static void init() { s_instance = new MandelbrotNavigator; }

        static void deinit() { delete s_instance; }

        static void setZoomCoef(ldouble coefficient);

        static const std::pair<ldouble, ldouble> fromScreenSpaceToMandelbrot(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        static void zoomIntoPoint(ldouble x, ldouble y);

    private:

        MandelbrotNavigator() = default;

        inline static MandelbrotNavigator* s_instance = nullptr;

        ldouble m_coef = 0.;
};

class Application
{
    public:

        static void init();

        static void deinit();

        static void runBenchmark();

        static void run();

    private:
        
        Application() = default;

        inline static Application* s_instance = nullptr;

        // window config
        const uint16_t m_windowWidth = 800u;
        const uint16_t m_windowHeight = 600u;
        const std::string m_windowName = "example2";        

        std::unique_ptr<sf::RenderWindow> m_window;
        sf::Image m_windowPixelBuffer;
};

int main()
{
    Application::init();
    Application::run();
    Application::deinit();

    return 0;
}


ScopedProfiler::ScopedProfiler() : m_creationTime{std::chrono::high_resolution_clock::now()} {  }

ScopedProfiler::~ScopedProfiler() 
{
    std::chrono::duration duration = std::chrono::high_resolution_clock::now() - m_creationTime;

    std::cout << "Passed time: " << duration.count() << std::endl;
}


void MandelbrotRenderer::setLeftEdge(ldouble leftEdge)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    s_instance->m_left = leftEdge;
}

void MandelbrotRenderer::setRightEdge(ldouble rightEdge)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    s_instance->m_right = rightEdge;
}

void MandelbrotRenderer::setTopEdge(ldouble topEdge)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    s_instance->m_top = topEdge;
}

void MandelbrotRenderer::setBottomEdge(ldouble bottomEdge)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    s_instance->m_bottom = bottomEdge;
}

ldouble MandelbrotRenderer::getLeftEdge()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    return s_instance->m_left;
}

ldouble MandelbrotRenderer::getRightEdge()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    return s_instance->m_right;
}

ldouble MandelbrotRenderer::getTopEdge()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    return s_instance->m_top;
}

ldouble MandelbrotRenderer::getBottomEdge()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    return s_instance->m_bottom;
}

template<MandelbrotRenderer::BatchingStrategy batchingStrategy = MandelbrotRenderer::BatchingStrategy::DISABLE_BATCHING,
         MandelbrotRenderer::ThreadingStrategy threadingStrategy = MandelbrotRenderer::ThreadingStrategy::DISABLE_THREADPOOL>
void MandelbrotRenderer::render(sf::Image& inoutImage)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    g_pool.resume();

    const auto imageSize = inoutImage.getSize();

    if constexpr (batchingStrategy == BatchingStrategy::DISABLE_BATCHING)
    {
        if constexpr (threadingStrategy == ThreadingStrategy::DISABLE_THREADPOOL)
        {
            for(uint32_t x = 0u; x < imageSize.x; ++x)
            {
                for(uint32_t y = 0u; y < imageSize.y; ++y)
                {   
                    inoutImage.setPixel(x, y, calculateMandelbrotColor(x, y, imageSize.x, imageSize.y));
                }                    
            }
        }

        if constexpr (threadingStrategy == ThreadingStrategy::ENABLE_THREADPOOL)
        {
            std::vector<ThreadPool::FunctionWrapper::Ptr> tasks;
            tasks.reserve(imageSize.x * imageSize.y);
            
            for(uint32_t x = 0u; x < imageSize.x; ++x)
            {
                for(uint32_t y = 0u; y < imageSize.y; ++y)
                {   
                    tasks.push_back
                    (
                        std::make_unique<ThreadPool::FunctionWrapper>
                        (
                            [x, y, imageSize, &inoutImage]() -> void 
                            {
                                inoutImage.setPixel(x, y, calculateMandelbrotColor(x, y, imageSize.x, imageSize.y));
                            }
                        )
                    );
                }                    
            }

            auto onComplete = g_pool.addTasksWithBarrier(std::move(tasks), []() -> bool { return true; });
            onComplete.get();
        }

        return;
    }

    if constexpr (batchingStrategy == BatchingStrategy::ENABLE_BATCHING)
    {
        if constexpr (threadingStrategy == ThreadingStrategy::DISABLE_THREADPOOL)
        {
            render<BatchingStrategy::DISABLE_BATCHING, ThreadingStrategy::DISABLE_THREADPOOL>(inoutImage);
        }

        if constexpr (threadingStrategy == ThreadingStrategy::ENABLE_THREADPOOL)
        {
            std::vector<ThreadPool::FunctionWrapper::Ptr> tasks;
            tasks.reserve(imageSize.x);

            uint32_t numBatches = 100u;

            uint32_t batchWidth = imageSize.x / numBatches;

            for(uint32_t batchID = 0u; batchID < numBatches; ++batchID)
            {
                tasks.push_back
                (
                    std::make_unique<ThreadPool::FunctionWrapper>
                    (
                        [batchID, batchWidth, imageSize, &inoutImage]() -> void 
                        {
                            for(uint32_t x = batchID * batchWidth; x < batchWidth * (batchID + 1); ++x)
                            {
                                for(uint32_t y = 0u; y < imageSize.y; ++y)
                                {
                                    inoutImage.setPixel(x, y, calculateMandelbrotColor(x, y, imageSize.x, imageSize.y));
                                }
                            }
                        }
                    )
                );
            }

            auto onComplete = g_pool.addTasksWithBarrier(std::move(tasks), []() -> bool { return true; });
            onComplete.get();
        }
    }
}

inline const ldouble MandelbrotRenderer::map(ldouble val, ldouble a0, ldouble b0, ldouble a1, ldouble b1)
{
    return a1 + (val - a0) * (b1 - a1) / (b0 - a0);
}

const sf::Color MandelbrotRenderer::calculateMandelbrotColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    ldouble a = map(static_cast<ldouble>(x), 0., static_cast<ldouble>(width), s_instance->m_left, s_instance->m_right);
    ldouble b = map(static_cast<ldouble>(y), 0., static_cast<ldouble>(height), s_instance->m_bottom, s_instance->m_top);

    ldouble ca = a, cb = b;

    uint32_t iteration = 0u;

    for(; iteration < s_instance->m_iterations; ++iteration)
    {
        ldouble aa = a*a - b*b, bb = 2 * a*b;
        a = aa + ca;
        b = bb + cb;

        if(a + b > 16.)
        {
            break;
        }
    }

    uint8_t brightness = static_cast<uint8_t>(map(static_cast<ldouble>(iteration), 0., s_instance->m_iterations, 0., 255.));

    uint32_t brightness2 = brightness * brightness;

    uint64_t brightness4 = brightness2 * brightness2;

    sf::Color result{ brightness, static_cast<uint8_t>(brightness2 % 255u), static_cast<uint8_t>(brightness4 % 255u) };

    return result;
}


void MandelbrotNavigator::setZoomCoef(ldouble coefficient)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    s_instance->m_coef = coefficient;
}

const std::pair<ldouble, ldouble> MandelbrotNavigator::fromScreenSpaceToMandelbrot(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    return 
    {
        MandelbrotRenderer::map(static_cast<ldouble>(x), 0., static_cast<ldouble>(width), MandelbrotRenderer::getLeftEdge(), MandelbrotRenderer::getRightEdge()),
        MandelbrotRenderer::map(static_cast<ldouble>(y), 0., static_cast<ldouble>(height), MandelbrotRenderer::getBottomEdge(), MandelbrotRenderer::getTopEdge())
    };
}

void MandelbrotNavigator::zoomIntoPoint(ldouble x, ldouble y)
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    ldouble invZoomCoef = 1. / s_instance->m_coef;

    // get width of prev edges
    ldouble  width = MandelbrotRenderer::getRightEdge() - MandelbrotRenderer::getLeftEdge();
    ldouble height = MandelbrotRenderer::getTopEdge() - MandelbrotRenderer::getBottomEdge();

    ldouble newHalfWidth = width * invZoomCoef * 0.5;
    ldouble newHalfHeight = height * invZoomCoef * 0.5;

    MandelbrotRenderer::setLeftEdge(x - newHalfWidth);
    MandelbrotRenderer::setRightEdge(x + newHalfWidth);
    MandelbrotRenderer::setTopEdge(y + newHalfHeight);
    MandelbrotRenderer::setBottomEdge(y - newHalfHeight);
}


void Application::init() 
{
    s_instance = new Application{};

    MandelbrotRenderer::init();
    MandelbrotNavigator::init();
    
    MandelbrotNavigator::setZoomCoef(4.);

    s_instance->m_window = std::make_unique<sf::RenderWindow>
    (
        sf::VideoMode(s_instance->m_windowWidth, s_instance->m_windowHeight),
        s_instance->m_windowName.c_str()
    );

    s_instance->m_windowPixelBuffer.create
    (
        s_instance->m_windowWidth,
        s_instance->m_windowHeight,
        sf::Color{0u}
    );
}

void Application::deinit()
{
    MandelbrotNavigator::deinit();  
    MandelbrotRenderer::deinit();

    delete s_instance;
}

void Application::runBenchmark()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    {
        ScopedProfiler p;

        MandelbrotRenderer::render
        <
            MandelbrotRenderer::BatchingStrategy::DISABLE_BATCHING,
            MandelbrotRenderer::ThreadingStrategy::DISABLE_THREADPOOL
        >
        (s_instance->m_windowPixelBuffer);
    }

    {
        ScopedProfiler p;

        MandelbrotRenderer::render
        <
            MandelbrotRenderer::BatchingStrategy::DISABLE_BATCHING,
            MandelbrotRenderer::ThreadingStrategy::ENABLE_THREADPOOL
        >
        (s_instance->m_windowPixelBuffer);
    }

    {
        ScopedProfiler p;

        MandelbrotRenderer::render
        <
            MandelbrotRenderer::BatchingStrategy::ENABLE_BATCHING,
            MandelbrotRenderer::ThreadingStrategy::ENABLE_THREADPOOL
        >
        (s_instance->m_windowPixelBuffer);
    }
}

void Application::run()
{
    ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

    runBenchmark();

    while (s_instance->m_window->isOpen())
    {
        static bool toRender = true;
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;

        while (s_instance->m_window->pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
            {
                s_instance->m_window->close();
            }

            if (event.type == sf::Event::MouseButtonReleased)
            {
                const auto mandelbrotXY = MandelbrotNavigator::fromScreenSpaceToMandelbrot(event.mouseButton.x, event.mouseButton.y, s_instance->m_windowWidth, s_instance->m_windowHeight);

                MandelbrotNavigator::zoomIntoPoint(mandelbrotXY.first, mandelbrotXY.second);

                toRender = true;
            }

            if(toRender)
            {
                ScopedProfiler p;

                MandelbrotRenderer::render
                <
                    MandelbrotRenderer::BatchingStrategy::ENABLE_BATCHING,
                    MandelbrotRenderer::ThreadingStrategy::ENABLE_THREADPOOL
                >
                (s_instance->m_windowPixelBuffer);

                toRender = false;
            }
        }

        sf::Texture texture;

        texture.create(s_instance->m_windowWidth, s_instance->m_windowHeight);
        texture.update(s_instance->m_windowPixelBuffer);

        sf::Sprite sprite(texture);

        s_instance->m_window->clear();
        s_instance->m_window->draw(sprite);
        s_instance->m_window->display();
    }
}