#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <utility>
#include <glm/vec2.hpp>

#include <boitatah/backend/vulkan/Vulkan.hpp>
#include <boitatah/backend/vulkan/Window.hpp>
#include <boitatah/modules/DescriptorSetManager.hpp>

#include <boitatah/modules/BackBufferDesc.hpp>
#include <boitatah/BoitatahEnums.hpp>
#include <boitatah/commands/CommandBuffer.hpp>

#include <boitatah/types/types.hpp>

#include <boitatah/buffers/BufferManager.hpp>

#include <boitatah/modules/ImageManager.hpp>
#include <boitatah/modules/RenderTargetManager.hpp>
#include <boitatah/modules/GPUResourceManager.hpp>
#include <boitatah/modules/MaterialManager.hpp>
#include <boitatah/modules/StageBaseMaterialManager.hpp>
#include <boitatah/modules/BackBuffer.hpp>
#include <boitatah/modules/Swapchain.hpp>
#include <boitatah/modules/BufferCamera.hpp>
#include <boitatah/modules/Camera.hpp>
#include <boitatah/collections.hpp>
#include <boitatah/scene/Scene.hpp>

namespace boitatah
{
    using namespace vk;
    using namespace buffer;


    class BackBufferManager;
    class Swapchain;

    ///Renderer Options
    ///Members
    ///     windowDimensions -> u32vec2:    initial window size.
    ///     appName -> const char *:        window title
    ///     debug -> bool:                  turns vulkan validation layers on/off
    ///     swapchainFormat -> IMAGE_FORMAT:the present image format.
    ///     backBufferDesc:                 render graph description. See BackBuffer.hpp   
    struct RendererOptions
    {
        glm::u32vec2 windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        IMAGE_FORMAT swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB;
        BackBufferDesc backBufferDesc;
    };

    ///Base Draw command target
    /// Holds the minimum data to render a SceneNode.
    struct RenderObject{
        Handle<Geometry> geometry;
        Handle<Material> material;
    };

    ///Base drawable
    typedef SceneTree<RenderObject>  RenderScene;

    //////////////////////////////////////////
    ///Renderer Class
    ///Provides render object management, GPU buffer management, Camera and Lights
    ///Provides a customizable rendergraph back buffer.
    ///Provides a RenderScene rendering function ( render_graph ),
    ///Exposes methods to compose a custom renderloop.
    ///@param   windowDimensions    initial window size.
    ///@param   appName window title
    ///@param   debug   turns vulkan validation layers on/off
    ///@param   swapchainFormat the present image format.
    ///@param   backBufferDesc  backbuffer render graph description. See BackBuffer.hpp
    //////////////////////////////////////////  
    class Renderer
    {
    public:
        //Constructor
        Renderer(RendererOptions options);

        //Destructor
        ~Renderer(void);

#pragma region Managers
        BufferManager&          getBufferManager();
        GPUResourceManager&     getResourceManager();
        MaterialManager&        getMaterialManager();
        DescriptorSetManager&   getDescriptorManager();
        Materials&              getMaterials();
#pragma endregion Managers

        ///Creates an orthographic camera. with a dedicated GPUBuffer.
        ///@param   lookAtTarget   the camera focal point.
        ///@param   position       camera initial position
        ///@param   far     distance of the far-plane
        ///@param   near    distance of the near-plane
        ///@param   aspect  camera aspect ration.
        ///@param   fov     camera field-of-view
        ///@returns a new Camera
        BufferedCamera create_camera(const CameraDesc& desc);

        /// TODO Temporary Light Function
        /// Attaches a LightArray to be used for in the render calls.
        /// Assigns the LightArray to be used as lighting in rendering.
        /// @param array the Handle for the light array to be used.
        void    set_light_array(const Handle<LightArray>& array);

        ///Creates a light array. See Lights.hpp
        ///@param size max light count.
        ///@returns a new LightArray Handle
        Handle<LightArray>  createLightArray(uint32_t size);

        ///Gets the reference to the referenced LightArray
        ///@param handle the LightArray Handle.
        LightArray&         getLightArray(Handle<LightArray> handle);
        
        ///Checks if the render window is closed.
        bool isWindowClosed();

        ///Waits for idle GPU.
        void waitIdle();

        ///Writes a DrawCommand to the CommandBufferWriter.
        ///@param writer    a command buffer writer that: has a Buffer attached and is writing.
        ///@param scene     a RenderScene to draw. This invokes only one draw call for 
        ///                 the Geometry and Material of the scene
        ///@param rendertarget  the destination Rendertarget.\
        ///@param frameIndex    the current frame index.
        template<typename T>
        void write_draw_command(CommandBufferWriter<T>      &writer,
                                RenderScene                 &scene,
                                const Handle<RenderTarget>  &rendertarget,
                                uint32_t                    frameIndex);

        ///Renders one SceneTree tree. See Scene.hpp
        ///Renders the tree according to the defined BackBuffer stages
        ///Sequentially calls render_graph_stage for each stage of the Backbuffer.
        ///Presents the results written to the present stage of the Backbuffer.
        /// TODO maybe template it to accept other types of SceneTree along a specific
        ///     SceneTree renderfunction.
        ///@param scene the SceneTree to be rendered.
        ///@param camera the camera to use for the render
        void render_tree(std::shared_ptr<RenderScene>      scene,
                                          BufferedCamera    &camera);

        ///Renders one RenderScene to one Stage of the BackBuffer.
        ///This function can be used to write renderloops.
        ///TODO maybe the render_tree function should be templatized 
        ///    or have a function parameter to 
        ///    customize the render_tree function instead of having to rewrite it.
        ///@param scene     the SceneTree to be rendered at this stage.
        ///@param camera    the camera
        ///@param stage     the renderstage to render to.
        ///@param wait_for_last_stage a synchronization semaphore to be waited for.
        ///@returns a synchronization semaphore, to be inputed for the next stage.
        VkSemaphore render_graph_stage(std::shared_ptr<RenderScene>     scene, 
                                                    BufferedCamera      &camera, 
                                                    Handle<RenderStage> stage,
                                                    VkSemaphore         wait_for_last_stage);

        ///Presents the RenderTarget to the swapchain/window.
        ///@param rendertarget  the rendertarget to present
        ///@param stage_wait    the seamphore to wait for.
        ///@param attachment_index  the attachment to display.
        void present_rendertarget(Handle<RenderTarget>    &rendertarget,
                                    VkSemaphore             stage_wait,
                                    uint32_t                attachment_index);
        
        ///Binds the vertex buffers of a Geometry to the command buffer writer.
        /// to be used when custom writing a renderloop.
        ///@param frame_index   the current frame number.
        ///@param geometry  the geometry handle to bind.
        ///@param indexed   whether to use vertex indexing.
        ///@param vertex_buffers    a list of which VERTEX_BUFFER_TYPEs to bind.
        ///@param writer    the buffer to write to, attached to a CommandBufferWriter. 
        void bind_vertexbuffers( uint32_t            frame_index, 
                                Handle<Geometry>    geometry, 
                                bool                indexed, 
                                std::vector<VERTEX_BUFFER_TYPE> vertex_buffers,
                                VkCommandBufferWriter           &writer);

    private:
        // Options Members
        RendererOptions m_options;

        // Base objects
        std::shared_ptr<BufferManager> m_bufferManager;
        std::shared_ptr<VkCommandBufferWriter> m_buffer_writer;
        std::shared_ptr<Swapchain> m_swapchain;
        std::shared_ptr<BackBufferManager> m_backBufferManager;
        std::shared_ptr<GPUResourceManager> m_resourceManager;
        std::shared_ptr<DescriptorSetManager> m_descriptorManager;
        std::shared_ptr<VulkanInstance> m_vk;
        std::shared_ptr<WindowManager> m_window;
        std::shared_ptr<MaterialManager> m_materialMngr;
        std::shared_ptr<ImageManager> m_imageManager;
        std::shared_ptr<RenderTargetManager> m_renderTargetManager;
        std::shared_ptr<Materials> m_baseMaterials;

        std::unique_ptr<Pool<LightArray>> m_lightpool;

        //TODO temp member
        Handle<LightArray> lights;

        void handleWindowResize();
        void createSwapchain();

        std::vector<std::shared_ptr<RenderScene>> 
        orderSceneNodes(const std::vector<std::shared_ptr<RenderScene>> &nodes) const;


        // Vulkan Instance
        void create_vulkan_instance();

        // Cleanup Functions
        void cleanup();
    };
}
//#endif // BOITATAH_RENDERER_HPP