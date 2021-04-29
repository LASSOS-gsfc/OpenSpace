/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2021                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <openspace/engine/configuration.h>

#include <openspace/documentation/documentation.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/lua/ghoul_lua.h>
#include <ghoul/lua/lua_helper.h>
#include <ghoul/misc/assert.h>
#include <optional>

namespace {
    // We can't use ${SCRIPTS} here as that hasn't been defined by this point
    constexpr const char* InitialConfigHelper =
                                               "${BASE}/scripts/configuration_helper.lua";

    struct [[codegen::Dictionary(Configuration)]] Parameters {
        // The SGCT configuration file that determines the window and view frustum
        // settings that are being used when OpenSpace is started
        std::optional<std::string> windowConfiguration [[codegen::key("SGCTConfig")]];

        // The scene description that is used to populate the application after startup.
        // The scene determines which objects are loaded, the startup time and other
        // scene-specific settings. More information is provided in the Scene
        // documentation. If the 'Asset' and the 'Profile' values are specified, the asset
        // is silently ignored
        std::optional<std::string> asset;

        // The profile that should be loaded at the startup. The profile determines which
        // assets are loaded, the startup time, keyboard shortcuts, and other settings.
        std::optional<std::string> profile;

        // This value names a list of scripts that get executed after initialization of
        // any scene. These scripts can be used for user-specific customization, such as a
        // global rebinding of keys from the default
        std::optional<std::vector<std::string>> globalCustomizationScripts;

        // A list of paths that are automatically registered with the file system. If a
        // key X is used in the table, it is then useable by referencing ${X} in all other
        // configuration files or scripts
        std::map<std::string, std::string> paths;

        // A list of all fonts that will automatically be loaded on startup. Each
        // key-value pair contained in the table will become the name and the file for a
        // font
        std::optional<std::map<std::string, std::string>> fonts;

        struct Logging {
            // List from logmanager.cpp::levelFromString
            enum class Level {
                Trace,
                Debug,
                Info,
                Warning,
                Error,
                Fatal,
                None
            };
            // The severity of log messages that will be displayed. Only messages of the
            // selected level or higher will be displayed. All levels below will be
            // silently discarded. The order of severities is:
            // Debug < Info < Warning < Error < Fatal < None.
            std::optional<Level> logLevel;

            // Determines whether error messages will be displayed immediately or if it is
            // acceptable to have a short delay, but being more performant. If the delay
            // is allowed ('true'), messages might get lost if the application crashes
            // shortly after a message was logged
            std::optional<bool> immediateFlush;

            // Per default, log messages are written to the console, the onscreen text,
            // and (if available) the Visual Studio output window. This table can define
            // other logging methods that will be used additionally
            std::optional<std::vector<ghoul::Dictionary>> logs
                [[codegen::reference("core_logfactory")]];

            // List from OpenspaceEngine::initialize
            enum class Verbosity {
                None,
                Minimal,
                Default,
                Full
            };
            // At startup, a list of system capabilities is created and logged. This value
            // determines how verbose this listing should be
            std::optional<Verbosity> capabilitiesVerbosity;
        };
        // Configurations for the logging of messages that are generated throughout the
        // code and are useful for debugging potential errors or other information
        std::optional<Logging> logging;

        // The file that will be created on startup containing the log of all Lua scripts
        // that are executed in the last session. Any existing file (including the results
        // from previous runs) will be silently overwritten
        std::optional<std::string> scriptLog;

        struct Documentation {
            // The path where the documentation files will be stored
            std::optional<std::string> path;
        };
        // Right now only contains the path where the documentation is written to
        std::optional<Documentation> documentation;

        // The countdown that the application will wait between pressing ESC and actually
        // shutting down. If ESC is pressed again in this time, the shutdown is aborted
        std::optional<float> shutdownCountdown [[codegen::greater(0.0)]];

        // If this is set to 'true', the name of the scene will be appended to the cache
        // directory, thus not reusing the same directory. This is useful in cases where
        // the same instance of OpenSpace is run with multiple scenes, but the caches
        // should be retained. This value defaults to 'false'
        std::optional<bool> perSceneCache;

        enum class Scaling {
            Window [[codegen::key("window")]],
            Framebuffer [[codegen::key("framebuffer")]]
        };
        // The method for scaling the onscreen text in the window. As the resolution of
        // the rendering can be different from the size of the window, the onscreen text
        // can either be scaled according to the window size ('window'), or the rendering
        // resolution ('framebuffer'). This value defaults to 'window'
        std::optional<Scaling> onScreenTextScaling;

        // List from RenderEngine::setRendererFromString
        enum class RenderingMethod {
            Framebuffer,
            ABuffer
        };
        // The renderer that is use after startup. The renderer 'ABuffer' requires support
        // for at least OpenGL 4.3
        std::optional<RenderingMethod> renderingMethod;

        // Toggles whether the master in a multi-application setup should be rendering or
        // just managing the state of the network. This is desired in cases where the
        // master computer does not have the resources to render a scene
        std::optional<bool> disableRenderingOnMaster;

        // Applies a global view rotation. Use this to rotate the position of the focus
        // node away from the default location on the screen. This setting persists even
        // when a new focus node is selected. Defined using roll, pitch, yaw in radians
        std::optional<glm::dvec3> globalRotation;

        // Applies a view rotation for only the master node, defined using roll, pitch yaw
        // in radians. This can be used to compensate the master view direction for tilted
        // display systems in clustered immersive environments
        std::optional<glm::dvec3> masterRotation;

        // Applies a global rotation for all screenspace renderables. Defined using roll,
        // pitch, yaw in radians
        std::optional<glm::dvec3> screenSpaceRotation;

        // If this value is set to 'true' the ingame console is disabled, locking the
        // system down against random access
        std::optional<bool> disableInGameConsole;

        // Toggles whether screenshots generated by OpenSpace contain the date when the
        // concrete OpenSpace instance was started. This value is enabled by default, but
        // it is advised to disable this value if rendering sessions of individual frames
        // pass beyond local midnight
        std::optional<bool> screenshotUseDate;

        struct HttpProxy {
            // Determines whether the proxy is being used
            std::optional<bool> activate;

            // The address of the http proxy
            std::string address;

            // The port of the http proxy
            int port [[codegen::inrange(0, 65536)]];

            enum class Authentication {
                Basic [[codegen::key("basic")]],
                Ntlm [[codegen::key("ntlm")]],
                Digest [[codegen::key("digest")]],
                Any [[codegen::key("any")]]
            };
            // The authentication method of the http proxy
            std::optional<Authentication> authentication;

            // The user of the http proxy
            std::optional<std::string> user;

            // The password of the http proxy
            std::optional<std::string> password;
        };
        // This defines the use for a proxy when fetching data over http. No proxy will be
        // used if this is left out
        std::optional<HttpProxy> httpProxy;

        struct OpenGLDebugContext {
            // Determines whether the OpenGL context should be a debug context
            bool activate;

            // Determines whether the OpenGL debug callbacks are performed synchronously.
            // If set to 'true' the callbacks are in the same thread as the context and in
            // the scope of the OpenGL function that triggered the message. The default
            // value is 'true'
            std::optional<bool> synchronous;

            // Individual OpenGL debug message identifiers
            struct Filter {
                // The identifier that is to be filtered
                int identifier;

                // Taken from ghoul::debugcontext.cpp
                enum class Source {
                    API,
                    WindowSystem [[codegen::key("Window System")]],
                    ShaderCompiler [[codegen::key("Shader Compiler")]],
                    ThirdParty [[codegen::key("Third Party")]],
                    Application,
                    Other,
                    DontCare [[codegen::key("Don't care")]]
                };
                // The source of the identifier to be filtered
                Source source;

                // Taken from ghoul::debugcontext.cpp
                enum class Type {
                    Error,
                    Deprecated,
                    Undefined,
                    Portability,
                    Performance,
                    Marker,
                    PushGroup [[codegen::key("Push group")]],
                    PopGroup [[codegen::key("Pop group")]],
                    Other,
                    DontCare [[codegen::key("Don't care")]]
                };
                // The type of the identifier to be filtered
                Type type;
            };
            // A list of OpenGL debug messages identifiers that are filtered
            std::optional<std::vector<Filter>> filterIdentifier;

            // A list of severities that can are filtered out
            enum class Severity {
                High,
                Medium,
                Low,
                Notification
            };
            // Determines the settings for the creation of an OpenGL debug context
            std::optional<std::vector<Severity>> filterSeverity;
        };
        // Determines the settings for the creation of an OpenGL debug context
        std::optional<OpenGLDebugContext> openGLDebugContext;

        // Determines whether the OpenGL state is checked after each OpenGL function call.
        // This will dramatically slow down the rendering, but will make finding OpenGL
        // errors easier. This defaults to 'false'
        std::optional<bool> checkOpenGLState;

        // Determines whether each OpenGL call that happens should be logged using the
        // 'TRACE' loglevel. This will bring the rendering to a crawl but provides useful
        // debugging features for the order in which OpenGL calls occur. This defaults to
        // 'false'
        std::optional<bool> logEachOpenGLCall;

        // This value determines whether the initialization of the scene graph should
        // occur multithreaded, that is, whether multiple scene graph nodes should
        // initialize in parallel. The only use for this value is to disable it for
        // debugging support
        std::optional<bool> useMultithreadedInitialization;

        // If this value is set to 'true', the launcher will not be shown and OpenSpace
        // will start with the provided configuration options directly. Useful in
        // multiprojector setups where a launcher window would be undesired
        std::optional<bool> bypassLauncher;

        // The URL that is pinged to check which version of OpenSpace is the most current
        // if you don't want this request to happen, this value should not be set at all
        std::optional<std::string> versionCheckUrl;

        struct LoadingScreen {
            // If this value is set to 'true', the loading screen will display a message
            // information about the current phase the loading is in
            std::optional<bool> showMessage;

            // If this value is set to 'true', the loading screen will display a list of
            // all of the nodes with their respective status (created, loaded,
            // initialized)
            std::optional<bool> showNodeNames;

            // If this value is set to 'true', the loading screen will contain a progress
            // bar that gives an estimate of the loading progression
            std::optional<bool> showProgressbar;
        };
        // Values in this table describe the behavior of the loading screen that is
        // displayed while the scene graph is created and initialized
        std::optional<LoadingScreen> loadingScreen;

        // List of profiles that cannot be overwritten by user
        std::optional<std::vector<std::string>> readOnlyProfiles;

        // Configurations for each module
        std::optional<std::map<std::string, ghoul::Dictionary>> moduleConfigurations;
    };
#include "configuration_codegen.cpp"
} // namespace

namespace openspace::configuration {

void parseLuaState(Configuration& configuration) {
    using namespace ghoul::lua;

    // Shorten the rest of this function
    Configuration& c = configuration;
    LuaState& s = c.state;

    // The sgctConfigNameInitialized is a bit special
    lua_getglobal(s, "sgctconfiginitializeString");
    c.sgctConfigNameInitialized = ghoul::lua::value<std::string>(
        s,
        ghoul::lua::PopValue::Yes
    );


    // The configuration file sets all values as global variables, so we need to pull them
    // into a table first so that we can pass that table to the dictionary constructor
    lua_newtable(s);

    // We go through all of the entries and lift them from global scope into the table on
    // the stack so that we can create a ghoul::Dictionary from this new table
    documentation::Documentation doc = codegen::doc<Parameters>();
    for (const documentation::DocumentationEntry& e : doc.entries) {
        lua_pushstring(s, e.key.c_str());
        lua_getglobal(s, e.key.c_str());
        lua_settable(s, -3);
    }


    ghoul::Dictionary d;
    ghoul::lua::luaDictionaryFromState(s, d);
    lua_settop(s, 0);
    const Parameters p = codegen::bake<Parameters>(d);

    c.windowConfiguration = p.windowConfiguration.value_or(c.windowConfiguration);
    c.asset = p.asset.value_or(c.asset);
    c.profile = p.profile.value_or(c.profile);
    c.globalCustomizationScripts =
        p.globalCustomizationScripts.value_or(c.globalCustomizationScripts);
    c.pathTokens = p.paths;
    c.fonts = p.fonts.value_or(c.fonts);
    c.scriptLog = p.scriptLog.value_or(c.scriptLog);
    c.versionCheckUrl = p.versionCheckUrl.value_or(c.versionCheckUrl);
    c.useMultithreadedInitialization =
        p.useMultithreadedInitialization.value_or(c.useMultithreadedInitialization);
    c.isCheckingOpenGLState = p.checkOpenGLState.value_or(c.isCheckingOpenGLState);
    c.isLoggingOpenGLCalls = p.logEachOpenGLCall.value_or(c.isLoggingOpenGLCalls);
    c.shutdownCountdown = p.shutdownCountdown.value_or(c.shutdownCountdown);
    c.shouldUseScreenshotDate = p.screenshotUseDate.value_or(c.shouldUseScreenshotDate);
    if (p.onScreenTextScaling.has_value()) {
        switch (*p.onScreenTextScaling) {
            case Parameters::Scaling::Window:
                c.onScreenTextScaling = "window";
                break;
            case Parameters::Scaling::Framebuffer:
                c.onScreenTextScaling = "framebuffer";
                break;
            default:
                throw ghoul::MissingCaseException();
        }
    }
    c.usePerSceneCache = p.perSceneCache.value_or(c.usePerSceneCache);
    c.isRenderingOnMasterDisabled =
        p.disableRenderingOnMaster.value_or(c.isRenderingOnMasterDisabled);
    c.globalRotation = p.globalRotation.value_or(c.globalRotation);
    c.masterRotation = p.masterRotation.value_or(c.masterRotation);
    c.screenSpaceRotation = p.screenSpaceRotation.value_or(c.screenSpaceRotation);
    if (p.renderingMethod.has_value()) {
        switch (*p.renderingMethod) {
            case Parameters::RenderingMethod::Framebuffer:
                c.renderingMethod = "Framebuffer";
                break;
            case Parameters::RenderingMethod::ABuffer:
                c.renderingMethod = "ABuffer";
                break;
            default:
                throw ghoul::MissingCaseException();
        }
    }
    c.isConsoleDisabled = p.disableInGameConsole.value_or(c.isConsoleDisabled);
    if (p.logging.has_value()) {
        if (p.logging->logLevel.has_value()) {
            switch (*p.logging->logLevel) {
                case Parameters::Logging::Level::Trace:
                    c.logging.level = "Trace";
                    break;
                case Parameters::Logging::Level::Debug:
                    c.logging.level = "Debug";
                    break;
                case Parameters::Logging::Level::Info:
                    c.logging.level = "Info";
                    break;
                case Parameters::Logging::Level::Warning:
                    c.logging.level = "Warning";
                    break;
                case Parameters::Logging::Level::Error:
                    c.logging.level = "Error";
                    break;
                case Parameters::Logging::Level::Fatal:
                    c.logging.level = "Fatal";
                    break;
                case Parameters::Logging::Level::None:
                    c.logging.level = "None";
                    break;
                default:
                    throw ghoul::MissingCaseException();
            }
        }

        c.logging.forceImmediateFlush =
            p.logging->immediateFlush.value_or(c.logging.forceImmediateFlush);
        c.logging.logs = p.logging->logs.value_or(c.logging.logs);
        if (p.logging->capabilitiesVerbosity.has_value()) {
            switch (*p.logging->capabilitiesVerbosity) {
                case Parameters::Logging::Verbosity::None:
                    c.logging.capabilitiesVerbosity = "None";
                    break;
                case Parameters::Logging::Verbosity::Minimal:
                    c.logging.capabilitiesVerbosity = "Minimal";
                    break;
                case Parameters::Logging::Verbosity::Default:
                    c.logging.capabilitiesVerbosity = "Default";
                    break;
                case Parameters::Logging::Verbosity::Full:
                    c.logging.capabilitiesVerbosity = "Full";
                    break;
                default:
                    throw ghoul::MissingCaseException();
            }
        }
    }

    if (p.documentation.has_value()) {
        c.documentation.path = p.documentation->path.value_or(c.documentation.path);
    }

    if (p.loadingScreen.has_value()) {
        const Parameters::LoadingScreen& l = *p.loadingScreen;
        c.loadingScreen.isShowingMessages =
            l.showMessage.value_or(c.loadingScreen.isShowingMessages);
        c.loadingScreen.isShowingProgressbar =
            l.showProgressbar.value_or(c.loadingScreen.isShowingProgressbar);
        c.loadingScreen.isShowingNodeNames =
            l.showNodeNames.value_or(c.loadingScreen.isShowingNodeNames);
    }

    c.moduleConfigurations = p.moduleConfigurations.value_or(c.moduleConfigurations);

    if (p.openGLDebugContext.has_value()) {
        const Parameters::OpenGLDebugContext& l = *p.openGLDebugContext;
        c.openGLDebugContext.isActive = l.activate;
        c.openGLDebugContext.isSynchronous = l.synchronous.value_or(
            c.openGLDebugContext.isSynchronous
        );
        if (l.filterIdentifier.has_value()) {
            for (const Parameters::OpenGLDebugContext::Filter& f : *l.filterIdentifier) {
                Configuration::OpenGLDebugContext::IdentifierFilter filter;
                filter.identifier = static_cast<unsigned int>(f.identifier);
                switch (f.source) {
                    case Parameters::OpenGLDebugContext::Filter::Source::API:
                        filter.source = "API";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::WindowSystem:
                        filter.source = "Window System";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::ShaderCompiler:
                        filter.source = "Shader Compiler";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::ThirdParty:
                        filter.source = "Third Party";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::Application:
                        filter.source = "Application";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::Other:
                        filter.source = "Other";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Source::DontCare:
                        filter.source = "Don't care";
                        break;
                    default:
                        throw ghoul::MissingCaseException();
                }
                switch (f.type) {
                    case Parameters::OpenGLDebugContext::Filter::Type::Error:
                        filter.type = "Error";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Deprecated:
                        filter.type = "Deprecated";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Undefined:
                        filter.type = "Undefined";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Portability:
                        filter.type = "Portability";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Performance:
                        filter.type = "Performance";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Marker:
                        filter.type = "Marker";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::PushGroup:
                        filter.type = "Push group";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::PopGroup:
                        filter.type = "Pop group";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::Other:
                        filter.type = "Other";
                        break;
                    case Parameters::OpenGLDebugContext::Filter::Type::DontCare:
                        filter.type = "Don't care";
                        break;
                    default:
                        throw ghoul::MissingCaseException();
                }

                c.openGLDebugContext.identifierFilters.push_back(filter);
            }
        }
        if (l.filterSeverity.has_value()) {
            for (Parameters::OpenGLDebugContext::Severity sev : *l.filterSeverity) {
                std::string severity;
                switch (sev) {
                    case Parameters::OpenGLDebugContext::Severity::High:
                        severity = "High";
                        break;
                    case Parameters::OpenGLDebugContext::Severity::Medium:
                        severity = "Medium";
                        break;
                    case Parameters::OpenGLDebugContext::Severity::Low:
                        severity = "Low";
                        break;
                    case Parameters::OpenGLDebugContext::Severity::Notification:
                        severity = "Notification";
                        break;
                    default:
                        throw ghoul::MissingCaseException();
                }
                c.openGLDebugContext.severityFilters.push_back(severity);
            }
        }
    }

    if (p.httpProxy.has_value()) {
        c.httpProxy.usingHttpProxy =
            p.httpProxy->activate.value_or(c.httpProxy.usingHttpProxy);
        c.httpProxy.address = p.httpProxy->address;
        c.httpProxy.port = static_cast<unsigned int>(p.httpProxy->port);
        if (p.httpProxy->authentication.has_value()) {
            switch (*p.httpProxy->authentication) {
                case Parameters::HttpProxy::Authentication::Basic:
                    c.httpProxy.authentication = "basic";
                    break;
                case Parameters::HttpProxy::Authentication::Ntlm:
                    c.httpProxy.authentication = "ntlm";
                    break;
                case Parameters::HttpProxy::Authentication::Digest:
                    c.httpProxy.authentication = "digest";
                    break;
                case Parameters::HttpProxy::Authentication::Any:
                    c.httpProxy.authentication = "any";
                    break;
                default:
                    throw ghoul::MissingCaseException();
            }
        }
        c.httpProxy.user = p.httpProxy->user.value_or(c.httpProxy.user);
        c.httpProxy.password = p.httpProxy->password.value_or(c.httpProxy.password);
    }

    c.readOnlyProfiles = p.readOnlyProfiles.value_or(c.readOnlyProfiles);
    c.bypassLauncher = p.bypassLauncher.value_or(c.bypassLauncher);
}

documentation::Documentation Configuration::Documentation = codegen::doc<Parameters>();

std::string findConfiguration(const std::string& filename) {
    using ghoul::filesystem::Directory;

    Directory directory = FileSys.absolutePath("${BIN}");

    while (true) {
        std::string fullPath = FileSys.pathByAppendingComponent(
            directory,
            filename
        );

        if (FileSys.fileExists(fullPath)) {
            // We have found the configuration file and can bail out
            return fullPath;
        }

        // Otherwise, we traverse the directory tree up
        Directory nextDirectory = directory.parentDirectory(
            ghoul::filesystem::Directory::AbsolutePath::Yes
        );

        if (directory.path() == nextDirectory.path()) {
            // We have reached the root of the file system and did not find the file
            throw ghoul::RuntimeError(
                "Could not find configuration file '" + filename + "'",
                "ConfigurationManager"
            );
        }
        directory = nextDirectory;
    }
}

Configuration loadConfigurationFromFile(const std::string& filename,
                                        const std::string& overrideScript)
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    ghoul_assert(FileSys.fileExists(filename), "File must exist");

    Configuration result;

    // If there is an initial config helper file, load it into the state
    if (FileSys.fileExists(absPath(InitialConfigHelper))) {
        ghoul::lua::runScriptFile(result.state, absPath(InitialConfigHelper));
    }

    // Load the configuration file into the state
    ghoul::lua::runScriptFile(result.state, filename);

    if (!overrideScript.empty()) {
        LDEBUGC("Configuration", "Executing Lua script passed through the commandline:");
        LDEBUGC("Configuration", overrideScript);
        ghoul::lua::runScript(result.state, overrideScript);
    }

    parseLuaState(result);

    return result;
}

} // namespace openspace::configuration
