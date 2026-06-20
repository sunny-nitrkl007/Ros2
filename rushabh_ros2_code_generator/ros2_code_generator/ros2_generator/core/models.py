from dataclasses import dataclass, field
from typing import Any

@dataclass

# Class: ApplicationInfo
# Purpose: See module docstring for this class's role in the generation pipeline.
class ApplicationInfo:
    name: str
    description: str
    version: str
    package_name: str
    maintainer_name: str
    maintainer_email: str
    license: str
    namespace: str = ""
    log_level: str = "info"
    use_sim_time: bool = False

@dataclass

# Class: CodeLayout
# Purpose: See module docstring for this class's role in the generation pipeline.
class CodeLayout:
    mode: str = "inline_user_sections"

@dataclass

# Class: GenerationOptions
# Purpose: See module docstring for this class's role in the generation pipeline.
class GenerationOptions:
    output_package_name: str
    output_directory: str
    enabled: bool = True
    language: str = "cpp"
    build_system: str = "ament_cmake"
    ros_distro: str = "humble"
    core_package: bool = True
    config: bool = True
    launch: bool = True
    tests: bool = True
    scripts: bool = True
    readme: bool = True
    generation_report: bool = True
    overwrite_generated: bool = True
    preserve_user_code: bool = True
    create_missing_user_stubs: bool = True

@dataclass

# Class: InterfaceMessage
# Purpose: See module docstring for this class's role in the generation pipeline.
class InterfaceMessage:
    name: str
    fields: list[dict]

@dataclass

# Class: InterfaceService
# Purpose: See module docstring for this class's role in the generation pipeline.
class InterfaceService:
    name: str
    request: list[dict]
    response: list[dict]

@dataclass

# Class: InterfaceModel
# Purpose: See module docstring for this class's role in the generation pipeline.
class InterfaceModel:
    messages: list[InterfaceMessage] = field(default_factory=list)
    services: list[InterfaceService] = field(default_factory=list)
    @property
    def enabled(self) -> bool:
        return bool(self.messages or self.services)

@dataclass

# Class: QoSProfile
# Purpose: See module docstring for this class's role in the generation pipeline.
class QoSProfile:
    name: str
    reliability: str
    durability: str
    history: str
    depth: int

@dataclass

# Class: TopicContract
# Purpose: See module docstring for this class's role in the generation pipeline.
class TopicContract:
    name: str
    topic_name: str
    message_type: str
    qos_profile: str
    description: str = ""

@dataclass

# Class: ServiceContract
# Purpose: See module docstring for this class's role in the generation pipeline.
class ServiceContract:
    name: str
    service_name: str
    service_type: str
    description: str = ""

@dataclass

# Class: ParameterSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class ParameterSpec:
    name: str
    type: str
    default: Any
    dynamic: bool = True
    description: str = ""
    min: Any = None
    max: Any = None
    allowed_values: list[Any] = field(default_factory=list)
    cpp_type: str = ""
    cpp_default: str = ""

@dataclass

# Class: PublisherSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class PublisherSpec:
    name: str
    topic_ref: str
    publish_mode: str = "timer"
    rate_hz: float | None = None
    rate_parameter: str | None = None
    logging: bool = True
    topic: TopicContract | None = None

@dataclass

# Class: WatchdogSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class WatchdogSpec:
    enabled: bool = False
    timeout_ms: int = 1000
    severity: str = "warn"
    idle_state: bool = True

@dataclass

# Class: SubscriberSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class SubscriberSpec:
    name: str
    topic_ref: str
    callback: str
    logging: bool = True
    watchdog: WatchdogSpec = field(default_factory=WatchdogSpec)
    topic: TopicContract | None = None

@dataclass

# Class: ServiceServerSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class ServiceServerSpec:
    name: str
    service_ref: str
    callback: str
    logging: bool = True
    service: ServiceContract | None = None

@dataclass

# Class: ServiceClientSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class ServiceClientSpec:
    name: str
    service_ref: str
    call_mode: str = "manual"
    wait_timeout_ms: int = 1000
    request_timeout_ms: int = 2000
    response_callback: str | None = None
    logging: bool = True
    service: ServiceContract | None = None

@dataclass

# Class: LoggingSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class LoggingSpec:
    enabled: bool = True
    level: str = "info"
    log_startup: bool = True
    log_shutdown: bool = True
    log_parameter_updates: bool = True

@dataclass

# Class: NodeSpec
# Purpose: See module docstring for this class's role in the generation pipeline.
class NodeSpec:
    name: str
    executable: str
    class_name: str
    type: str = "regular"
    parameter_group: str | None = None
    logging: LoggingSpec = field(default_factory=LoggingSpec)
    parameters: list[ParameterSpec] = field(default_factory=list)
    publishers: list[PublisherSpec] = field(default_factory=list)
    subscribers: list[SubscriberSpec] = field(default_factory=list)
    services: list[ServiceServerSpec] = field(default_factory=list)
    clients: list[ServiceClientSpec] = field(default_factory=list)

@dataclass

# Class: ApplicationModel
# Purpose: See module docstring for this class's role in the generation pipeline.
class ApplicationModel:
    app: ApplicationInfo
    generation: GenerationOptions
    code_layout: CodeLayout
    interfaces: InterfaceModel
    qos_profiles: dict[str, QoSProfile]
    topics: dict[str, TopicContract]
    services: dict[str, ServiceContract]
    nodes: list[NodeSpec]
    dependencies: list[str] = field(default_factory=list)
