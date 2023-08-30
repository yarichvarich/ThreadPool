#include <ThreadPool.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>

using ldouble = long double;

ThreadPool g_pool{5u};

class ScopedProfiler
{
    public:

        ScopedProfiler() { /* creationTime = ... currentTime */ }
        ~ScopedProfiler() { /* std::*/ }

    private:

        // creation time
};

class MandelbrotRenderer
{
    public:

        static void init() { s_instance = new MandelbrotRenderer{}; }
        static void deinit() { delete s_instance; }

        static void setLeftEdge(ldouble leftEdge)
        {
            //ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

            s_instance->m_left = leftEdge;
        }

        static void setRightEdge(ldouble rightEdge)
        {
            //ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

            s_instance->m_right = rightEdge;
        }

        static void setTopEdge(ldouble topEdge)
        {
            //ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

            s_instance->m_top = topEdge;
        }

        static void setBottomEdge(ldouble bottomEdge)
        {
            //ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

            s_instance->m_bottom = bottomEdge;
        }

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

        template<BatchingStrategy batchingStrategy = BatchingStrategy::DISABLE_BATCHING, ThreadingStrategy threadingStrategy = ThreadingStrategy::DISABLE_THREADPOOL>
        static void render(sf::Image& inoutImage)
        {
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

                    for(uint32_t x = 0u; x < imageSize.x; ++x)
                    {
                        tasks.push_back
                        (
                            std::make_unique<ThreadPool::FunctionWrapper>
                            (
                                [x, imageSize, &inoutImage]() -> void 
                                {
                                    for(uint32_t y = 0u; y < imageSize.y; ++y)
                                    {
                                        inoutImage.setPixel(x, y, calculateMandelbrotColor(x, y, imageSize.x, imageSize.y));
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

    private:

        MandelbrotRenderer() = default;
        
        static const ldouble map(ldouble val, ldouble a0, ldouble b0, ldouble a1, ldouble b1)
        {
            return a1 + (val - a0) * (b1 - a1) / (b0 - a0);
        }

        static const sf::Color calculateMandelbrotColor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            ldouble a = map(static_cast<ldouble>(x), 0., static_cast<ldouble>(width), -2., 2.);
            ldouble b = map(static_cast<ldouble>(y), 0., static_cast<ldouble>(height), -2., 2.);

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

            uint16_t brightness2 = brightness * brightness;

            uint32_t brightness4 = brightness2 * brightness2;

            sf::Color result{ brightness, brightness2 % 255u, brightness4 % 255u};

            return result;
        }

        inline static MandelbrotRenderer* s_instance = nullptr;
        
        uint32_t m_iterations = 5000u;

        ldouble m_left = -2.;
        ldouble m_right = 2.;
        ldouble m_top = 2.;
        ldouble m_bottom = -2.;
};

class Application
{
    public:

        static void init() 
        {
            s_instance = new Application{};

            MandelbrotRenderer::init();

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

        static void deinit()
        {
            MandelbrotRenderer::deinit();

            delete s_instance;
        }

        static void run()
        {
            //ALWAYS_ASSERT(s_instance != nullptr && "<-- assert missed init call");

            MandelbrotRenderer::render
                    <
                        MandelbrotRenderer::BatchingStrategy::ENABLE_BATCHING,
                        MandelbrotRenderer::ThreadingStrategy::ENABLE_THREADPOOL
                    >(s_instance->m_windowPixelBuffer);

            while (s_instance->m_window->isOpen())
            {
                // check all the window's events that were triggered since the last iteration of the loop
                sf::Event event;

                while (s_instance->m_window->pollEvent(event))
                {
                    // "close requested" event: we close the window
                    if (event.type == sf::Event::Closed)
                    {
                        s_instance->m_window->close();
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