# x265参数
struct
{
	int cpuid; // CPU识别标识
	/*== 并行处理特性 ==*/
	int frameNumThreads; // 并行处理的帧数量
	const char *numaPools; // NUMA节点线程池配置
	int bEnableWavefront; // 启用波前并行处理
	int bDistributeModeAnalysis; // 使用多线程测量CU模式成本
	int bDistributeMotionEstimation; // 使用多线程进行运动估计
	/*== 日志记录特性 ==*/
	int bLogCuStats; // 启用CU的分析和记录【已废弃】
	int bEnablePsnr; // 启用PSNR的测量和报告
	int bEnableSsim; // 启用SSIM的测量和报告
	int logLevel; // 编码器输出的日志级别
	int csvLogLevel; // CSV日志记录级别
	const char *csvfn; // CSV日志文件名

	/*== 内部图像规格 ==*/
	int internalBitDepth; // 内部编码器的位深度
	int internalCsp; // 内部图像的颜色空间
	uint32_t fpsNum; // 帧率的分子
	uint32_t fpsDenom; // 帧率的分母
	int sourceWidth; // 源图像宽度
	int sourceHeight; // 源图像高度
	int interlaceMode; // 源图像的交错类型 隔行扫描
	int totalFrames;   // 要编码的总帧数，根据用户输入(--frames)和(--seek)计算得到，若从管道读取输入则为0，将用于两遍率控制，因此存储在param中

	/*== Profile / Tier / Level ==*/
	int levelIdc;  // 最低解码器要求的级别。默认为0，表示编码器会自动检测级别。若指定，则编码器将尝试使编码规格达到指定级别。若编码器无法达到指定级别，则发出警告并输出实际的解码器要求。若请求的要求级别高于实际级别，则会输出实际要求级别。值表示为整数，例如级别"5.1"表示为51，级别"5.0"表示为50。
	int bHighTier; // 若levelIdc被指定（非零），此标志将区分Main(0)和High(1)层次。默认为Main层次(0)。
	int uhdBluray; // 启用UHD Blu-ray兼容性支持。若指定，编码器将尝试修改/设置编码规格。若编码器无法实现，则此选项将关闭。
	int maxNumReferences;	  // P或B帧可以使用的最大L0参考帧数。影响解码图像缓冲区的大小。该值越大，越多的参考帧可用于运动搜索，从而提高大多数视频的压缩效率，但性能开销较大。值必须在1和16之间，默认为3。
	int bAllowNonConformance; // 允许libx265发出不符合严格级别要求的HEVC比特流。默认为false。

	/*== Bitstream Options ==*/
	int bRepeatHeaders;				 // 标志，指示是否在每个关键帧输出VPS、SPS和PPS头。默认为false。
	int bAnnexB;					 // 标志，指示编码器是否应该在NAL单元之前输出起始码（Annex B格式）或长度（文件格式）。默认为true，Annex B格式。Muxer应将此值设置为正确的值。
	int bEnableAccessUnitDelimiters; // 标志，指示编码器是否在每个存取单元的开头发出访问单元分隔符NAL。默认为false。 是否添加aud
	int bEmitHRDSEI;				 // 启用缓冲区管理SEI和图片时序SEI以传递HRD参数。默认为禁用。
	int bEmitInfoSEI;				 // 启用带有流头的用户数据SEI，描述编码器版本、构建信息和参数。这对于调试非常有用，但可能会干扰回归测试。默认启用。
	int decodedPictureHashSEI;		 // 启用为每个编码帧生成SEI消息，其中包含三个重构图片平面的哈希值。大多数解码器将验证这些哈希值与它们生成的重构图像，并报告任何不匹配。这实质上是一个调试特性。哈希类型为MD5(1)，CRC(2)，Checksum(3)。默认为0，表示无。
	int bEnableTemporalSubLayers;	 // 在编码时启用时间子层，用于向编码的NAL单元传递temporalId。输出比特流可以在基本时间层(layer 0)提取，其帧率约为一半，或者在较高时间层(layer 1)提取，解码序列中的所有帧。
	
	/*== GOP structure and slice type decisions (lookahead) ==*/
	int bOpenGOP;		   // 启用开放式GOP（Open GOP），默认为true。
	int keyframeMin;	   // I帧间隔最小值 非IDR
	int keyframeMax;	   // Iz帧最大间隔。默认为250。
	int bframes;		   // 连续B帧的最大数量。当b-adapt为0且keyframMax大于bframes时，lookahead会在每个P帧之间发出一个固定的`bframes`个B帧的模式。b-adapt为1时，lookahead在大多数情况下忽略bframes的值。b-adapt为2时，bframes的值决定在两个方向上执行搜索（POC距离），使lookahead的计算负载呈二次增加。该值越高，lookahead可能连续使用的B帧越多，通常可以提高压缩效率。默认为3，最大值为16。
	int bFrameAdaptive;	   // 设置lookahead的操作模式。对于b-adapt 0，GOP结构根据keyframeMax和bframes的值是固定的。对于b-adapt 1，使用轻量级的lookahead来选择B帧的位置。对于b-adapt 2（trellis），执行维特比B路径选择。
	int bBPyramid;		   // 启用将大于2个B帧的mini-GOP中的中间B帧用作周围B帧的运动参考。这提高了小的性能开销的压缩效率。被引用的B帧在速率控制中被视为介于B帧和P帧之间。默认启用。
	int bFrameBias;		   // 该值加在lookahead中B帧的成本估计中。它可以是正值（使B帧的成本估计看起来更便宜，从而使lookahead选择更多的B帧），或者是负值，这样lookahead就会选择更多的P帧。默认为0，没有限制。
	int lookaheadDepth;	   // 必须在lookahead中排队的帧数，才能进行片段决策。增加此值直接增加编码延迟。队列越长，lookahead可以更优地进行片段决策，特别是在b-adapt 2中启用cu-tree分析的情况下。默认为40帧，最大为250。
	int lookaheadSlices;   // 使用多个工作线程对lookahead中的每个帧的估计成本。对于bFrameAdaptive 2，大多数帧成本估计将以批处理模式执行，同时进行多个成本估计，此时忽略lookaheadSlices的值。该参数越高，帧成本将越不准确（因为在切片边界上丢失了上下文），这将导致B帧和场景切换的决策不太准确。默认为0（禁用），1与0相同。最大值为16。
	int scenecutThreshold; // 用于决定lookahead如何检测场景切换的阈值。建议使用默认值（40）。
	int bIntraRefresh;	   // 使用从一侧到另一侧移动的一列帧内块来替换关键帧，从而“刷新”图像。实际上，关键帧被“分散”在许多帧上。
	
	/*== Coding Unit (CU) definitions ==*/
	uint32_t maxCUSize;	  // 最大CU宽度和高度（以像素为单位）。大小必须是64、32或16。尺寸越大，x265可以更有效地编码低复杂度的区域，从而在大分辨率下大大提高压缩效率。尺寸越小，由于行数增加，波前和帧并行性将变得更加有效。默认为64。同一进程中的所有编码器必须使用相同的maxCUSize，直到关闭所有编码器并调用x265_cleanup()以重置该值。
	uint32_t minCUSize;	  // 最小CU宽度和高度（以像素为单位）。大小必须是64、32、16或8。默认为8。同一进程中的所有编码器必须使用相同的minCUSize。
	int bEnableRectInter; // 启用矩形运动预测分区（垂直和水平），在64x64到8x8的所有CU深度上可用。默认为禁用。
	int bEnableAMP;		  // 启用不对称运动预测。在64、32和16的CU深度上，可以使用25%/75%的分割分区，分别在上、下、右、左方向上。对于某些材料，这可以提高压缩效率，但需要额外的分析。必须启用bEnableRectInter才能使用此特性。默认禁用。
	/*== Residual Quadtree Transform Unit (TU) definitions ==*/
	uint32_t maxTUSize;			// 最大TU宽度和高度（以像素为单位）。大小必须是32、16、8或4。尺寸越大，残差在DCT变换时可以更有效地压缩，但计算量会增加。
	uint32_t tuQTMaxInterDepth; // 允许在编码块之外额外递归的残差四叉树深度，用于inter coded blocks。此值必须在1和4之间。该值越大，残差在DCT变换时可以更有效地压缩，但计算量会大大增加。
	uint32_t tuQTMaxIntraDepth; // 允许在编码块之外额外递归的残差四叉树深度，用于intra coded blocks。此值必须在1和4之间。该值越大，残差在DCT变换时可以更有效地压缩，但计算量会大大增加。
	uint32_t limitTU;			// 为了避免递归到更高TU深度，启用对于inter coded blocks的早期退出决策。默认为0。
	int rdoqLevel;			  // 设置在量化过程中使用的速率失真分析程度。0表示没有速率失真优化。级别1使用率失真成本找到每个层次的最佳舍入值（并允许启用psy-rdoq）。级别2使用率失真成本对每个4x4编码组（包括在编码组内部的群组的成本信号）进行decimate决策。psy-rdoq在RDOQ为级别2时的效果较差。默认为0。
	int bEnableSignHiding;	  // 隐式信号化每个变换单元最后一个系数的符号位。这在每个TU节省一个比特，代价是找出可以用最小失真切换的系数。默认启用。
	int bEnableTransformSkip; // 允许在效率更高时，将帧内编码块直接编码为残差而不经过DCT变换。检查块是否从该选项中受益会导致性能开销。默认禁用。
	int noiseReductionIntra;  // 帧内CU的降噪强度，范围0到2000。0表示禁用。
	int noiseReductionInter;  // 帧间CU的降噪强度，范围0到2000。0表示禁用。
	const char *scalingLists; // 量化缩放列表，用于定义HEVC支持的6个缩放列表，分别用于Y、Cb、Cr的帧内和帧间预测。
	int bEnableConstrainedIntra; // 启用受限帧内预测，使帧内预测使用来自帧间预测的输入样本。
	int bEnableStrongIntraSmoothing; // 启用强烈帧内平滑，适用于32x32块，当参考样本是平坦的时候。
	uint32_t maxNumMergeCand; // 最大合并候选数，在帧间分析时考虑的合并候选数。
	uint32_t limitReferences; // 基于先前运动搜索结果限制每个CU使用的运动参考。
	uint32_t limitModes; // 使用来自4个子CU的代价度量限制每个CU分析的模式数。
	int searchMethod; // 运动估计搜索方法(DIA, HEX, UMH, STAR, SEA, FULL)。
	int subpelRefine; // 子像素细化的程度，值在0到X265_MAX_SUBPEL_LEVEL之间，用于子像素运动估计。
	int searchRange; // 全像素运动搜索允许的最大距离。
	int bEnableTemporalMvp; // 启用时域运动矢量预测的可用性。
	int bEnableWeightedPred; // 在P帧中启用加权预测，允许加权分析影响片段决策和运动补偿。
	int bEnableWeightedBiPred; // 在B帧中启用加权双向预测。
	int bSourceReferenceEstimation; // 启用源像素运动估计。
	int bEnableLoopFilter; // 启用去块滤波器，减少边缘块效应。
	int deblockingFilterTCOffset; // 去块滤波器tC偏移。
	int deblockingFilterBetaOffset; // 去块滤波器Beta偏移。
	int bEnableSAO; // 启用样本自适应偏移循环滤波器，通过直方图分析调整重构样本值。
	int bSaoNonDeblocked; // 选择SAO处理去块滤波器边界像素的方法。
	int rdLevel; // 速率失真优化级别，值在1到6之间，越高压缩效率越好但性能较差。
	int bEnableEarlySkip; // 启用早期跳过决策，避免在可能跳过的块中分析额外的模式。
	int bEnableRecursionSkip; // 启用早期CU大小决策，避免递归到更高的深度。
	int bEnableFastIntra; // 使用更快速的方法查找最佳帧内预测模式。
	int bEnableTSkipFast; // 启用更快速的跳过变换/量化判断。
	int bCULossless; // CU无损标志，启用时对每个CU选择最佳模式进行无损编码。
	int bIntraInBFrames; // 在B帧中尝试编码帧内模式。
	int rdPenalty; // 对非帧内片段的32x32帧内块应用惩罚，0禁用，1启用小惩罚，2启用完全惩罚。
	double psyRd; // 心理视觉速率失真强度，影响模式选择对能量的保留程度。
	double psyRdoq; // 心理视觉优化的量化强度。
	int bEnableRdRefine; // 执行基于量化参数的RD细化，计算最优的四叉树划分。
	int analysisReuseMode; // 分析信息复用模式，用于减少编码器的处理量。
	const char *analysisReuseFileName; // analysisReuseMode保存/加载的文件名。
	int bLossless; // 启用无损编码，绕过缩放、变换、量化和循环滤波过程。
	int cbQpOffset; // Cb色度残差的QP偏移。
	int crQpOffset; // Cr色度残差的QP偏移。
	/*码流控制*/
	struct
	{
		int rateControlMode; // 码率控制模式，必须是X265_RC_METHODS枚举值之一。 X265_RC_ABR、X265_RC_CQP、X265_RC_CRF
		int qp; // X265_RC_CQP OP值
		int bitrate; // X265_RC_ABR 码率
		double qCompress; // 量化器曲线压缩因子，用于根据预测复杂性权衡帧的量化器。值在0.6到1之间。
		double ipFactor; // I/P帧和P/B帧之间的QP偏移。
		double pbFactor; // P/B帧之间的QP偏移。
		double rfConstant; // X265_RC_CRF 默认28 0-51
		int qpStep; // 帧间最大QP差值。
		int aqMode; // 启用自适应量化。此模式在所有CTU中分配可用比特，为低复杂度区域分配更多比特。
		double aqStrength; // AQ对低细节CTU的偏向强度。
		int vbvMaxBitrate; // VBV缓冲区的最大填充速率。
		int vbvBufferSize; // VBV缓冲区的大小（以kb为单位）。
		double vbvBufferInit; // 播放开始前VBV缓冲区必须填充的比例或固定值。
		int cuTree; // 启用CU树形率控制，优化跨帧传播的CU。
		double rfConstantMax; // X265_RC_CRF模式下由VBV引起的最大CRF值。
		double rfConstantMin; // X265_RC_CRF模式下由VBV引起的最小CRF值。
		/*2-pass*/
		int bStatWrite; // 在多遍编码中启用将统计数据写入stat输出文件。
		int bStatRead; // 在多遍编码中启用从stat输入文件加载数据。
		const char *statFileName; // 两遍编码输出/输入统计文件的文件名。

		double qblur; // 临时模糊的量化因子。
		double complexityBlur; // 临时模糊的复杂度。
		int bEnableSlowFirstPass; // 在多遍率控制中启用缓慢且更详细的第一遍编码。
		int zoneCount; // 码率控制区域数量。
		x265_zone *zones; // 码率控制区域数组。
		const char *lambdaFileName; // 指定包含两个浮点数数组的文本文件，复制到x265_lambda_tab和x265_lambda2_tab中。
		int bStrictCbr; // 在CBR模式下启用更严格的比特率偏差检查。
		uint32_t qgSize; // CU粒度自适应量化的最小CU尺寸（Quantization Group）。
		int bEnableGrain; // 如果设置了tune grain，则内部启用。
		int qpMax; // QP的上限。
		int qpMin; // QP的下限。
		int bEnableConstVbv; // 如果设置了tune grain，则内部启用。
	}
	rc;
	struct
	{
		int aspectRatioIdc; // VUI中添加的宽高比IDC。默认为0，表示宽高比未指定。
		int sarWidth; // 只有在aspectRatioIdc设置为X265_EXTENDED_SAR时，VUI中才会添加的样本宽高比宽度。
		int sarHeight; // 只有在aspectRatioIdc设置为X265_EXTENDED_SAR时，VUI中才会添加的样本宽高比高度。
		int bEnableOverscanInfoPresentFlag; // 启用VUI中的超扫描信息标志。默认为false。
		int bEnableOverscanAppropriateFlag; // 只有在bEnableOverscanInfoPresentFlag设置时，VUI中才会添加的超扫描适合标志。默认为false。
		int bEnableVideoSignalTypePresentFlag; // 启用VUI中的视频信号类型标志。默认为false。
		int videoFormat; // 源视频的视频格式。0 = component, 1 = PAL, 2 = NTSC, 3 = SECAM, 4 = MAC, 5 =未指定视频格式。
		int bEnableVideoFullRangeFlag; // 视频全范围标志，指示亮度和色度信号的黑电平和范围。默认为false。
		int bEnableColorDescriptionPresentFlag; // 启用VUI中的颜色描述标志。默认为false。
		int colorPrimaries; // 源主色度的色度坐标。默认为2。
		int transferCharacteristics; // 源图像的光电转换特性。默认为2。
		int matrixCoeffs; // 从红、蓝和绿原色导出亮度和色度信号的矩阵系数。默认为2。
		int bEnableChromaLocInfoPresentFlag; // 启用VUI中的色度位置信息标志。默认为false。
		int chromaSampleLocTypeTopField; // 顶场的色度采样位置类型。默认为0。
		int chromaSampleLocTypeBottomField; // 底场的色度采样位置类型。默认为0。
		int bEnableDefaultDisplayWindowFlag; // 启用VUI中的默认显示窗口标志。默认为false。
		int defDispWinLeftOffset; // 默认显示窗口的左偏移。
		int defDispWinRightOffset; // 默认显示窗口的右偏移。
		int defDispWinTopOffset; // 默认显示窗口的上偏移。
		int defDispWinBottomOffset; // 默认显示窗口的下偏移。
	} vui;
	const char *masteringDisplayColorVolume; // 主显示器色彩容量信息
	uint16_t maxCLL;						 // 最大内容光亮度
	uint16_t maxFALL;						 // 最大帧平均光亮度
	uint16_t minLuma;						 // 输入源图像的最小亮度
	uint16_t maxLuma;						 // 输入源图像的最大亮度
	int log2MaxPocLsb;						 // 图像序列的最大图片顺序计数值的对数表示
	int bEmitVUITimingInfo;					 // 发送VUI定时信息，一个可选的VUI字段
	int bEmitVUIHRDInfo;					 // 发送HRD定时信息
	unsigned int maxSlices;					 // 图像的最大切片数
	int bOptQpPPS;							 // 根据前一GOP的统计信息优化PPS中的QP
	int bOptRefListLengthPPS;				 // 根据前一GOP的统计信息优化PPS中的参考列表长度
	int bMultiPassOptRPS;					 // 在多遍模式下在SPS中存储常用的参考图片集合
	double scenecutBias;					 // 用于场景切换检测的帧的inter cost和intra cost之间的百分比差异，默认值为5
	int lookaheadThreads;					 // 使用多个专用于预测的工作线程，而不是与帧编码器共享工作线程。通过指定数量的工作线程创建一个专用的预测线程池。可选择的范围从0到可用于编码的硬件线程数量的一半。使用太多的预测线程可能会损害帧编码器的资源并降低性能，默认值为0（禁用）。
	int bOptCUDeltaQP;						 // 优化CU级别的QP，以在rd level > 4时为帧信号化一致的deltaQPs
	int analysisMultiPassRefine;			 // 基于存储的分析信息在多遍速率控制中进行分析优化
	int analysisMultiPassDistortion;		 // 基于存储的失真数据在多遍速率控制中进行分析优化
	int bAQMotion;							 // 基于相对运动的自适应量化
	int bSsimRd;							 // 基于SSIM的残差除法归一化方案的RDO，用于在对CTU进行分析期间进行模式选择，可以在客观质量指标SSIM和PSNR方面获得显著的增益
	double dynamicRd;						 // 在由于vbv而导致比特率下降的点增加RD，默认为0
	int bEmitHDRSEI;						 // 启用发射包含HDR特定参数的HDR SEI（Supplemental Enhancement Information）。当指定了max-cll、max-fall或主显示器信息时，自动启用。默认情况下禁用。
	int bHDROpt;							 // 启用HDR/WCG内容的亮度和色度偏移。默认情况下禁用。
	int analysisReuseLevel;					 // 保存/加载分析重用模式中存储/重用信息的级别。值越高，存储/重用的信息越多。默认值为5。
	int bLimitSAO;							 // 通过在SAO过程中提前终止来限制采样自适应偏移滤波器的计算，基于inter预测模式、CTU空间域相关性以及亮度和色度之间的关系
	const char *toneMapFile;				 // 包含色调映射信息的文件
	int bDhdr10opt;							 // 仅在IDR帧和色调映射信息发生变化时插入色调映射信息
	int bCTUInfo;							 // 确定x265如何对通过API接收的内容信息作出反应
	int bUseRcStats;						 // 使用来自pic_in的速率控制统计信息（如果可用）
	int scaleFactor;						 // 用于分析保存模式下将输入视频缩小的因子。默认值为0。
	int intraRefine;						 // 在加载模式下启用帧内细化
	int interRefine;						 // 在加载模式下启用帧间细化
	int mvRefine;							 // 在加载模式下启用运动矢量细化
	uint32_t maxLog2CUSize;					 // 最大CTU尺寸的对数表示
	uint32_t maxCUDepth;					 // 相对于配置深度的实际CU深度
	uint32_t unitSizeDepth;					 // 相对于最大变换尺寸的CU深度
	uint32_t num4x4Partitions;				 // 最大CU尺寸中的4x4单元数
	int bUseAnalysisFile;					 // 指定分析模式是否使用文件进行数据重用
	FILE *csvfpt;							 // 用于CSV日志的文件指针
	int forceFlush;							 // 强制从编码器刷新帧
	int bEnableSplitRdSkip;					 // 当分割CU的rdCost之和大于非分割CU的rdCost时，允许跳过分割RD分析
	int bDisableLookahead;					 // 禁用预测
	int bLowPassDct;						 // 使用低通滤波的子频带DCT逼近，这种DCT逼近计算量较小，结果接近标准DCT
	double vbvBufferEnd;					 // 在将所有指定帧插入解码缓冲区后，必须保留的解码缓冲区的一部分。如果小于1，则最终可用的缓冲区为vbv-end * vbvBufferSize。否则，它被解释为最终可用的缓冲区（以kbits为单位）。默认值为0（禁用）。
	double vbvEndFrameAdjust;				 // 要调整的qp值，以达到最终解码缓冲区清空的帧。指定为总帧数的一部分。默认为0。
	int bMVType;							 // 通过API获取的MV信息的重用
	int bCopyPicToFrame;					 // 允许编码器在Frame中拥有x265_picture的平面副本
} x265_param;
