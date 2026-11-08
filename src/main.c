#include <SDL3/SDL_pixels.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vectors.h>
#include <transformations.h>
#include <ray.h>
#include <shapes.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <webgpu/webgpu.h>
#include <sdl3webgpu.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct {
	int width, height;
	vec4 *pixels;
} screen;


struct program_state {
	SDL_Window *window;
	screen pixels;

	int frame_accumulation_count;
	float cam_yaw;
	float cam_pitch;
	vec3 cam_pos;

	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;
	WGPUSurface surf;
	WGPURenderPipeline pipeline;
	WGPUBuffer tri_buffer;

	WGPUBuffer resolution_buffer;


	WGPUBindGroupLayout bind_group_layout;
	WGPUBindGroup bind_group;
	WGPUPipelineLayout layout;
};

void on_webgpu_adapter_request_end(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView view, void * userdata1, void *userdata2) {
	*(WGPUAdapter*)userdata1 = adapter;
	*(_Bool*)userdata2 = 1;
}

void webgpu_dev_callback(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView view, void * userdata1, void *userdata2) {
	*(WGPUDevice*)userdata1 = device;
	*(_Bool*)userdata2 = 1;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	printf("ray: %zi, hitinfo: %zi\n", sizeof(ray), sizeof(hitinfo));
	*appstate = malloc(sizeof(struct program_state));
	struct program_state *state = (struct program_state*)*appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to init SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	state->window = SDL_CreateWindow("pathtracer", WIDTH, HEIGHT, 0);
	if(state->window == NULL) {
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetWindowRelativeMouseMode(state->window, true);

	WGPUInstanceDescriptor desc = {0};
	desc.nextInChain = NULL;

#ifndef WEBGPU_BACKEND_EMSCRIPTEN
	WGPUInstance instance = wgpuCreateInstance(&desc);
#else
	WGPU instance = wgpuCreateInstance(NULL);
#endif


	if(!instance) {
		fprintf(stderr, "Failed to create webgpu instance\n");
		return SDL_APP_FAILURE;
 	}

	WGPURequestAdapterOptions adapter_opts = {0};
	adapter_opts.nextInChain = NULL;
	state->surf = SDL_GetWGPUSurface(instance, state->window);
	adapter_opts.compatibleSurface = state->surf;

	WGPURequestAdapterCallbackInfo adapter_callback_info = {0};
	adapter_callback_info.nextInChain = NULL;
	adapter_callback_info.callback = on_webgpu_adapter_request_end;
	adapter_callback_info.userdata1 = &state->adapter;
	_Bool done = 0;
	adapter_callback_info.userdata2 = &done;
	wgpuInstanceRequestAdapter(instance, &adapter_opts, adapter_callback_info);

	#ifdef __EMSCRIPTEN__
    while (!done) {
        emscripten_sleep(100);
    }
	#endif // __EMSCRIPTEN__

	if(!state->adapter) {
		fprintf(stderr, "Failed to create webgpu adapter\n");
		return SDL_APP_FAILURE;
	}

	wgpuInstanceRelease(instance);
	
	WGPUDeviceDescriptor dev_descriptor = {0};
	dev_descriptor.nextInChain = NULL;
	WGPUStringView dev_label;
	dev_label.data = "Dev";
	dev_label.length = 3;
	dev_descriptor.label = dev_label;
	dev_descriptor.requiredFeatureCount = 0;
	dev_descriptor.requiredLimits = NULL;
	dev_descriptor.defaultQueue.nextInChain = NULL;
	dev_descriptor.defaultQueue.label = dev_label;

	_Bool dev_done = 0;
	WGPURequestDeviceCallbackInfo dev_callback_info = {0};
	dev_callback_info.nextInChain = NULL;
	dev_callback_info.callback = webgpu_dev_callback;
	dev_callback_info.userdata1 = &state->device;
	dev_callback_info.userdata2 = &dev_done;

	wgpuAdapterRequestDevice(state->adapter, &dev_descriptor, dev_callback_info);

	#ifdef __EMSCRIPTEN__
    while (!dev_done) {
        emscripten_sleep(100);
    }
	#endif

	if(!state->device) {
		fprintf(stderr, "Failed to create webgpu device\n");
		return SDL_APP_FAILURE;
	}

	state->queue = wgpuDeviceGetQueue(state->device);

	WGPUSurfaceConfiguration config = {0};
	config.nextInChain = NULL;
	config.width = WIDTH;
	config.height = HEIGHT;

	WGPUSurfaceCapabilities caps;
	caps.nextInChain = NULL;
	wgpuSurfaceGetCapabilities(state->surf, state->adapter, &caps);

	config.format = caps.formats[0];
	config.viewFormatCount = 0;
	config.viewFormats = NULL;
	config.usage = WGPUTextureUsage_RenderAttachment;
	config.device = state->device;
	config.presentMode = WGPUPresentMode_Fifo;
	config.alphaMode = WGPUCompositeAlphaMode_Auto;

	wgpuSurfaceConfigure(state->surf, &config);



	/*printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			x.columns[0][0],x.columns[1][0],x.columns[2][0],x.columns[3][0],
			x.columns[0][1],x.columns[1][1],x.columns[2][1],x.columns[3][1],
			x.columns[0][2],x.columns[1][2],x.columns[2][2],x.columns[3][2],
			x.columns[0][3],x.columns[1][3],x.columns[2][3],x.columns[3][3]
			);*/
	/*
	state->cam_yaw = -0.956666;
	state->cam_pitch = 0.248333;
	state->cam_pos = (vec3){1.937445, -0.791324, 0.774603};

	SDL_Surface *win_surface = SDL_GetWindowSurface(state->window);

	state->pixels.width = win_surface->w;
	state->pixels.height = win_surface->h;
	state->pixels.pixels = malloc(state->pixels.width * state->pixels.height * sizeof(vec4));

	state->frame_accumulation_count = 0;


	return SDL_APP_CONTINUE;*/


	float vertex_data[] = {
		-5, -5,

		+5, -5,

		+0.0, +5 };

	WGPUBufferDescriptor buffer_desc = {0};
	buffer_desc.nextInChain = NULL;
	buffer_desc.label.data = "gpubuffer";
	buffer_desc.label.length = WGPU_STRLEN;
	buffer_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
	buffer_desc.size = sizeof(vertex_data);
	buffer_desc.mappedAtCreation = false;
	state->tri_buffer = wgpuDeviceCreateBuffer(state->device, &buffer_desc);

	wgpuQueueWriteBuffer(state->queue, state->tri_buffer, 0, vertex_data, buffer_desc.size);

	WGPUVertexBufferLayout vertex_buffer_layout = {0};

	WGPUVertexAttribute position_attribute;
	position_attribute.shaderLocation = 0;
	position_attribute.format = WGPUVertexFormat_Float32x2;
	position_attribute.offset = 0;


	vertex_buffer_layout.attributeCount = 1;
	vertex_buffer_layout.attributes = &position_attribute;
	vertex_buffer_layout.arrayStride = 2 * sizeof(float);
	vertex_buffer_layout.stepMode = WGPUVertexStepMode_Vertex;

	

	WGPUShaderModuleDescriptor shader_mod_desc = {0};
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	WGPUShaderSourceWGSL shader_code_desc = {0};
	shader_code_desc.chain.next = NULL;
	shader_code_desc.chain.sType = WGPUSType_ShaderSourceWGSL;

	shader_code_desc.code.data = R"(
		@vertex
		fn vs_main(@location(0) in_vertex_position: vec2f) -> @builtin(position) vec4f {
    		return vec4f(in_vertex_position, 0.0, 1.0);
		}

		struct uniformData {
			camMatrix: mat4x4<f32>,
			camPos: vec4f,
			resolution: vec2f
		};

		struct sphere {
			position: vec4f,
			radius: f32
		};

		struct ray {
			origin: vec4f,
			direction: vec4f
		}

fn ray_sphere_intersection(r : ray, s : sphere) -> bool {
	var oc : vec4f = s.position - r.origin;
    var a : f32 = dot(r.direction, r.direction);
    var b : f32 = -2.0 * dot(r.direction, oc);
    var c : f32 = dot(oc, oc) - s.radius*s.radius;
    var discriminant : f32 = b*b - 4*a*c;

    if (discriminant < 0) {
		return false;
    } else {
        /*out.distance =  (-b - sqrt(discriminant) ) / (2.0*a);
		out.normal = v3_norm(r.direction * out.distance + r.origin - s.position);
		out.position = r.direction * out.distance + r.origin;
		out.hit = 1;*/
		return true;
    }
	//return out;
}


		@group(0) @binding(0) var<uniform> uData: uniformData;
		@fragment
		fn fs_main(@builtin(position) pos: vec4f) -> @location(0) vec4f {
						/*float ndc_x = ((float)x / state->pixels.width - 0.5) * 2;
			float ndc_y = ((float)y / state->pixels.height - 0.5) * 2;
			vec4 ray_dir = m4_v4_mul(rot_matrix, (vec4){ndc_x, ndc_y, 1, 0});*/
		var s : sphere;
			s.position = vec4(0, 0, -4, 0);
			s.radius = 1.0;

			var ndcX : f32 = (pos.x / uData.resolution.x - 0.5) * 2.0;
			var ndcY : f32 = (pos.y / uData.resolution.y - 0.5) * 2.0;
			var rayDir : vec4f = uData.camMatrix * vec4f(normalize(vec3f(ndcX, ndcY, 1.0)), 0.0);

			var r: ray;
			r.origin = uData.camPos;
			r.direction = rayDir;

			var hit : f32 = f32(ray_sphere_intersection(r, s));

		    //return vec4f(normalize(rayDir.xyz), 1);
		    return vec4f(hit, hit, hit, 1);
		}
	)";
	shader_code_desc.code.length = WGPU_STRLEN;

	shader_mod_desc.nextInChain = &shader_code_desc.chain;

	WGPUShaderModule shader_module = wgpuDeviceCreateShaderModule(state->device, &shader_mod_desc);

	WGPURenderPipelineDescriptor render_pipe_desc = {0};
	render_pipe_desc.nextInChain = NULL;
	render_pipe_desc.vertex.bufferCount = 1;
	render_pipe_desc.vertex.buffers = &vertex_buffer_layout;

	render_pipe_desc.vertex.module = shader_module;
	render_pipe_desc.vertex.entryPoint.data = "vs_main";
	render_pipe_desc.vertex.entryPoint.length = 7;
	render_pipe_desc.vertex.constantCount = 0;
	render_pipe_desc.vertex.constants = NULL;
	
	render_pipe_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
	render_pipe_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
	render_pipe_desc.primitive.frontFace = WGPUFrontFace_CCW;
	render_pipe_desc.primitive.cullMode = WGPUCullMode_None;


	WGPUFragmentState fragment_state = {0};
	fragment_state.module = shader_module;
	fragment_state.entryPoint.data = "fs_main";
	fragment_state.entryPoint.length = 7;
	fragment_state.constantCount = 0;
	fragment_state.constants = NULL;

	render_pipe_desc.fragment = &fragment_state;
	render_pipe_desc.depthStencil = NULL;

	WGPUBlendState blend_state = {0};

	blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
	blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
	blend_state.color.operation = WGPUBlendOperation_Add;

	WGPUColorTargetState color_target = {0};


	color_target.format = caps.formats[0];
	color_target.blend = &blend_state;
	color_target.writeMask = WGPUColorWriteMask_All;

	fragment_state.targetCount = 1;
	fragment_state.targets = &color_target;
	
	render_pipe_desc.multisample.count = 1;
	render_pipe_desc.multisample.mask = ~0u;
	render_pipe_desc.multisample.alphaToCoverageEnabled = false;
	
	WGPUBufferDescriptor res_uni_desc = {0};
	res_uni_desc.size = 24 * sizeof(float);
	res_uni_desc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
	res_uni_desc.mappedAtCreation = false;

	state->resolution_buffer = wgpuDeviceCreateBuffer(state->device, &res_uni_desc);
	wgpuQueueWriteBuffer(state->queue, state->resolution_buffer, 0, &(mat4){0}, sizeof(float)*16);
	wgpuQueueWriteBuffer(state->queue, state->resolution_buffer, 20 * sizeof(float), (float[]){WIDTH, HEIGHT}, sizeof(float)*2);

	WGPUBindGroupLayoutEntry binding_layout = {0};
	binding_layout.buffer.nextInChain = NULL;
	binding_layout.buffer.type = WGPUBufferBindingType_Undefined;
	binding_layout.buffer.hasDynamicOffset = false;

/*	binding_layout.sampler.nextInChain = NULL;
	binding_layout.sampler.type = WGPUSamplerBindingType_Undefined;

	binding_layout.storageTexture.nextInChain = NULL;
	binding_layout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
	binding_layout.storageTexture.format = WGPUTextureFormat_Undefined;
	binding_layout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

	binding_layout.texture.nextInChain = NULL;
	binding_layout.texture.multisampled = false;
	binding_layout.texture.sampleType = WGPUTextureSampleType_Undefined;
	binding_layout.texture.viewDimension = WGPUTextureViewDimension_Undefined;*/

	binding_layout.binding = 0;

	binding_layout.visibility = WGPUShaderStage_Fragment;

	binding_layout.buffer.type = WGPUBufferBindingType_Uniform;
	binding_layout.buffer.minBindingSize = 24 * sizeof(float);

	WGPUBindGroupLayoutDescriptor bind_group_lay_desc = {0};
	bind_group_lay_desc.nextInChain = NULL;
	bind_group_lay_desc.entryCount = 1;
	bind_group_lay_desc.entries = &binding_layout;
	state->bind_group_layout = wgpuDeviceCreateBindGroupLayout(state->device, &bind_group_lay_desc);

	WGPUPipelineLayoutDescriptor layout_desc = {0};
	layout_desc.nextInChain = NULL;
	layout_desc.bindGroupLayoutCount = 1;
	layout_desc.bindGroupLayouts = &state->bind_group_layout;
	state->layout = wgpuDeviceCreatePipelineLayout(state->device, &layout_desc);

	WGPUBindGroupEntry binding[2] = {{0}, {0}};
	binding[0].nextInChain = NULL;
	binding[0].binding = 0;
	binding[0].buffer = state->resolution_buffer;
	binding[0].offset = 0;
	binding[0].size = 24 * sizeof(float);



	WGPUBindGroupDescriptor bind_group_desc = {0};
	bind_group_desc.nextInChain = NULL;
	bind_group_desc.layout = state->bind_group_layout;

	bind_group_desc.entryCount = 1;
	bind_group_desc.entries = binding;
	state->bind_group = wgpuDeviceCreateBindGroup(state->device, &bind_group_desc);


	render_pipe_desc.layout = state->layout;

	state->pipeline = wgpuDeviceCreateRenderPipeline(state->device, &render_pipe_desc);
	wgpuShaderModuleRelease(shader_module);

	state->cam_yaw = 0;
	state->cam_pitch = 0;



	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	struct program_state *state = (struct program_state*)appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;
		case SDL_EVENT_KEY_DOWN:
			switch(event->key.key) {
				case SDLK_P:
					SDL_SavePNG(SDL_GetWindowSurface(state->window), "screenshot.png");

				case SDLK_C:
					printf("{%f, %f, %f}, %f, %f", state->cam_pos[0], state->cam_pos[1], state->cam_pos[2], state->cam_yaw, state->cam_pitch);
					break;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			state->cam_yaw += event->motion.xrel / WIDTH;
			state->cam_pitch -= event->motion.yrel / HEIGHT;
			state->cam_pitch = fmax(state->cam_pitch, -M_PI/2);
			state->cam_pitch = fmin(state->cam_pitch, M_PI/2);

			state->frame_accumulation_count = 0;
			break;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	struct program_state *state = (struct program_state*)appstate;
	if(state->window != NULL) {
		SDL_DestroyWindow(state->window);
	}

	wgpuBufferRelease(state->tri_buffer);
	wgpuBufferRelease(state->resolution_buffer);
	wgpuSurfaceRelease(state->surf);
	wgpuAdapterRelease(state->adapter);
	wgpuDeviceRelease(state->device);
	wgpuQueueRelease(state->queue);

	wgpuBindGroupRelease(state->bind_group);
	wgpuPipelineLayoutRelease(state->layout);
	wgpuBindGroupLayoutRelease(state->bind_group_layout);
	wgpuRenderPipelineRelease(state->pipeline);
}

vec3 environment(vec3 direction) {
	return (vec3){0.6, 0.6, 0.8};
}


vec3 sample_env_light(vec3 direction) {
	vec3 ground_col = {0.2, 0.2, 0.2};
	vec3 sky_col = (vec3){0.8, 0.8, 1} * 0.8f;
	sky_col += sky_col * (1-(float)fmin(1-direction[1], 2) / 2) * 1.f;
	vec3 ground = ground_col * (float)(direction[1] > 0);
	vec3 sky = sky_col * (float)!(direction[1] > 0);

	vec3 sun_dir = v3_norm((vec3){-1, -1, -0.2});

	float sun = pow(v3_dot(sun_dir, direction) * 0.5 + 0.5, 300) * 10;

	return (vec3){0};
	return ground + sky + sun;
}

vec3 trace(ray r, const sphere *spheres, int sphere_count, uint32_t *rng_seed) {
	int bounce_count = 4;
	ray current_ray = r;
	current_ray.color = (vec3){1, 1, 1, 1};
	current_ray.incoming_light = (vec3){0, 0, 0, 0};
	for(int bounce = 0; bounce < bounce_count; ++bounce) {
		float min_t = INFINITY;
		const sphere *hit_sphere = NULL;
		material mat;
		hitinfo hit_info;
		for(int i = 0; i < sphere_count; ++i) {
			hitinfo info = ray_sphere_intersection(current_ray, spheres[i]);
			if(!info.hit) continue;
			if(info.distance < min_t) {
				hit_sphere = spheres + i;
				min_t = info.distance;
				hit_info = info;
				mat = spheres[i].mat;
			}
		}

		if(min_t != INFINITY) {
			material_bounce(mat, &current_ray, hit_info, rng_seed);
			current_ray.origin = hit_info.position;
			/*
			current_ray.origin = hit_info.position;
			if((hit_sphere == spheres || hit_sphere == spheres + 1)) {
			vec3 reflection_fuzz = rand_in_sphere(rng_seed);
			vec3 reflection = v3_norm(current_ray.direction - 2 * v3_dot(current_ray.direction, hit_info.normal) * hit_info.normal + reflection_fuzz * 0.3f);
			if(v3_dot(reflection, hit_info.normal) < 0) {
				reflection = v3_norm(current_ray.direction - 2 * v3_dot(current_ray.direction, hit_info.normal) * hit_info.normal - reflection_fuzz * 0.3f);
			}
			current_ray.direction = reflection;
			vec3 emitted_light = hit_sphere->emit;
			incoming_light += emitted_light * ray_color;
			ray_color *= hit_sphere->color;
			} else {
				if(hash(rng_seed) > 0.5)
				{
					current_ray.direction = v3_norm(rand_vec(rng_seed) + hit_info.normal);
			vec3 emitted_light = hit_sphere->emit;
			incoming_light += emitted_light * ray_color;
			ray_color *= hit_sphere->color;
				} else
					current_ray.direction = v3_norm(current_ray.direction - 2 * v3_dot(current_ray.direction, hit_info.normal) * hit_info.normal + rand_in_sphere(rng_seed) * 0.1f);
			vec3 emitted_light = hit_sphere->emit;
			incoming_light += emitted_light * ray_color;
			//ray_color *= hit_sphere->color;
			}
			//return 0.5*(normal + 1);*/
		} else {
			//light *= environment(current_ray.direction);
			current_ray.incoming_light += sample_env_light(current_ray.direction) * current_ray.color;
			break;
		};
	}
	current_ray.incoming_light[3] = 1;
	return current_ray.incoming_light;
}

vec3 trace_tris(ray *r, const tri *tris, int tri_count, uint32_t *rng_seed) {
	int bounce_count = 10;
	ray current_ray = *r;
	current_ray.color = (vec3){1, 1, 1, 1};
	current_ray.incoming_light = (vec3){0, 0, 0, 0};
	for(int bounce = 0; bounce < bounce_count; ++bounce) {
		float min_t = INFINITY;
		material mat;
		hitinfo hit_info;
		for(int i = 0; i < tri_count; ++i) {
			hitinfo info = ray_tri_intersection(&current_ray, tris[i]);
			if(!info.hit) continue;
			if(info.distance < min_t) {
				min_t = info.distance;
				hit_info = info;
				mat = tris[i].mat;
			}
		}

		if(min_t != INFINITY) {
			material_bounce(mat, &current_ray, hit_info, rng_seed);
			current_ray.origin = hit_info.position;
		} else {
			current_ray.incoming_light += sample_env_light(current_ray.direction) * current_ray.color;
			break;
		};
	}
	current_ray.incoming_light[3] = 1;
	return current_ray.incoming_light;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	struct program_state *state = (struct program_state*)appstate;

	float aspect_ratio = (float)state->pixels.width / state->pixels.height;

	const _Bool * const kbd_state = SDL_GetKeyboardState(NULL);

	if(kbd_state[SDL_SCANCODE_W]) {
		//state->cam_pos[2] += 0.01;
		state->cam_pos[0] -= sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] -= cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] += sin(state->cam_pitch) * 0.1;

		state->frame_accumulation_count = 0;
	}

	if(kbd_state[SDL_SCANCODE_S]) {
		//state->cam_pos[2] -= 0.01;
		//
		state->cam_pos[0] += sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] += cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] -= sin(state->cam_pitch) * 0.1;
		state->frame_accumulation_count = 0;
	}


	const tri tris[12] = {
		// left face
		{
			{-1, 1, -1},
			{-1, -1, 1},
			{-1, -1, -1},
			{
				.color = (vec3){1, 0, 0},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			{-1, 1, -1},
			{-1, 1, 1},
			{-1, -1, 1},
			{
				.color = (vec3){1, 0, 0},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},


		// right face
		{
			{1, -1, -1},
			{1, -1, 1},
			{1, 1, -1},
			{
				.color = (vec3){0, 1, 0},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			{1, -1, 1},
			{1, 1, 1},
			{1, 1, -1},
			{
				.color = (vec3){0, 1, 0},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		// back face
		{
			{-1, -1, -1},
			{1, 1, -1},
			{-1, 1, -1},
			{
				.color = (vec3){0, 0, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			{-1, -1, -1},
			{1, -1, -1},
			{1, 1, -1},
			{
				.color = (vec3){0, 0, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		// top face
		{
			{-1, -1, -1},
			{-1, -1, 1},
			{1, -1, 1},
			{
				.color = (vec3){1, 1, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			{1, -1, 1},
			{1, -1, -1},
			{-1, -1, -1},
			{
				.color = (vec3){1, 1, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		// bottom face
		{
			{-1, 1, -1},
			{1, 1, 1},
			{-1, 1, 1},
			{
				.color = (vec3){1, 1, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			{-1, 1, -1},
			{1, 1, -1},
			{1, 1, 1},
			{
				.color = (vec3){1, 1, 1},
				.emissive_color = (vec3){0,0,0},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}

		},

		// top face
		{
			(vec3){-1, -1.1, -1} * 0.75f,
			(vec3){-1, -1.1, 1} * 0.75f,
			(vec3){1, -1.1, 1} * 0.75f,
			{
				.color = (vec3){0, 0, 0},
				.emissive_color = (vec3){2,2,2},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},

		{
			(vec3){1, -1.1, 1} * 0.75f,
			(vec3){1, -1.1, -1} * 0.75f,
			(vec3){-1, -1.1, -1} * 0.75f,
			{
				.color = (vec3){0, 0, 0},
				.emissive_color = (vec3){2,2,2},
				.specular_color = (vec3){0,0,0},
				.roughness = 0,
				.specularity = 0
			}
		},
	};
	const int tricount = 12;

	WGPUStringView dev_label;
	dev_label.data = "tex_view_desc";
	dev_label.length = 13;



	WGPUSurfaceTexture tex;
	wgpuSurfaceGetCurrentTexture(state->surf, &tex);

	WGPUTextureViewDescriptor tex_view_desc = {0};
	tex_view_desc.nextInChain = NULL;
	tex_view_desc.label = dev_label;
	tex_view_desc.format = wgpuTextureGetFormat(tex.texture);
	tex_view_desc.dimension = WGPUTextureViewDimension_2D;
	tex_view_desc.baseMipLevel = 0;
	tex_view_desc.mipLevelCount = 1;
	tex_view_desc.baseArrayLayer = 0;
	tex_view_desc.arrayLayerCount = 1;
	tex_view_desc.aspect = WGPUTextureAspect_All;
	WGPUTextureView target_view = wgpuTextureCreateView(tex.texture, &tex_view_desc);

	WGPUCommandEncoderDescriptor encoder_desc = {0};
	encoder_desc.nextInChain = NULL;
	encoder_desc.label = dev_label;
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(state->device, &encoder_desc);

	WGPURenderPassDescriptor render_pass_desc = {0};
	render_pass_desc.nextInChain = NULL;

	WGPURenderPassColorAttachment col_attach = {0};

	col_attach.view = target_view;
	col_attach.resolveTarget = NULL;
	col_attach.loadOp = WGPULoadOp_Clear;
	col_attach.storeOp = WGPUStoreOp_Store;
	col_attach.clearValue = (WGPUColor){ 0.9, 0.1, 0.2, 1.0 };
	#ifndef WEBGPU_BACKEND_WGPU
	col_attach.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	#endif
	render_pass_desc.depthStencilAttachment = NULL;
	render_pass_desc.timestampWrites = NULL;

	render_pass_desc.colorAttachmentCount = 1;
	render_pass_desc.colorAttachments = &col_attach;

	WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);

	struct {
		mat4 a;
		vec4 pos;
		vec4 b;
	} uniform_data;

	mat4 rot_matrix = gen_rotation_matrix(state->cam_yaw, state->cam_pitch, 0);
	uniform_data.a = rot_matrix;
	uniform_data.b = (vec4){WIDTH, HEIGHT, 0, 0};
	uniform_data.pos = state->cam_pos;

	wgpuQueueWriteBuffer(state->queue, state->resolution_buffer, 0, &uniform_data, sizeof(uniform_data));
	
	wgpuRenderPassEncoderSetPipeline(render_pass, state->pipeline);
	wgpuRenderPassEncoderSetVertexBuffer(render_pass, 0, state->tri_buffer, 0, wgpuBufferGetSize(state->tri_buffer));

	wgpuRenderPassEncoderSetBindGroup(render_pass, 0, state->bind_group, 0, NULL);

	wgpuRenderPassEncoderDraw(render_pass, 3, 1, 0, 0);



	wgpuRenderPassEncoderEnd(render_pass);
	wgpuRenderPassEncoderRelease(render_pass);



	//wgpuCommandEncoderInsertDebugMarker(encoder, dev_label);

	WGPUCommandBufferDescriptor buf_desc = {0};
	buf_desc.nextInChain = NULL;
	buf_desc.label = dev_label;
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &buf_desc);

	wgpuQueueSubmit(state->queue, 1, &command);

	wgpuCommandBufferRelease(command);



	wgpuSurfacePresent(state->surf);

	#ifndef WEBGPU_BACKEND_WGPU
    wgpuTextureRelease(tex.texture);
	#endif 

	wgpuTextureViewRelease(target_view);
	return SDL_APP_CONTINUE;
}
