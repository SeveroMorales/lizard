        y  N      g????????02?f?eL???f$8?N?uZ            (?/?`N} V?Z&?8 ?;?E?c? ??}?X?刱??.?t?j  ?jR R O ?k??p???BCe?Q*?G?L	??i??(7??2?nsF?}?Vva?8?g?6p1:?J?*܈˴ꔯ?i??S3?s????W?z???W??lA?l^f?(????Č????Cp?\N??o8G?`?H
*???i?q???b/???`$?zq?z#5??????ҡ???F???_1R???3??lq??tg<?9.?q6N(;??????h2?%!?(?$L??.?F?C??ʑYG/=?^?&????(?=?fԒ?ㇸR??Ɠ?|9??M]?iN?=??k?? ߬?!?.?R???7?W??˼?Ma??;gV???m?.(?j??%C3? B @B0??@AJb?1??I?$?v??1?O???JfUՎ?\?OV???עֵ? ?????фq`?/%?Z??O???}?i?????E??#Ӌ??ޖU?G?Vb???G?#??4?AR??~R?(7L/2??g?<?)? :d?????V??6YJP?Z2???(?yp????ʗ???G?B??J??񱼊?+;P)?`?˔ ?74?6??????~Є??A?? Z??v???wV?-?]?@    y     <  M      k    ??????+ۣ???????Q?=,S              ?     0		error('pygobject does not work with python3')
    ?     H  W     p   ????^!Zh?y?????I0O?z??              .  `   <		install_dir : join_paths(get_option('libdir'), 'gplugin')
    ?       7     ?   ????.???
P???)?l???ޖH              '  W   subdir('tests')
         t  ?     ?   ??????s!??h֜?x?	?<$+?H            (?/? ?]   ?  ?   y	PYTHON3 = dependency('python3-embed', required: false)
	if not .found()
	')
	endif
 Td7?    ?     @  ?     ?   ????#{?}?????t8?`??_??              ?  ?   4	gplugin_python = shared_library('gplugin-python3',
    ?     ?  ?     ?   ??????)r???@????¸??嗊            (?/? ?? ?  ?  ?   L	PYTHON3 = dependency('python3-embed', version: '>=3.5.0', required: false)
  ?  ?   6	  D   =	PYGOBJECTgobject-30.0')
  ??c?E?`?%'3    r    ?  ?     :   ????6?=??=??????^??z?\7f            (?/?`o? &?R:Pk? o* (?P????Z?KQ?w;S?a????r?Skmie?%??Ĭ?*??x?C???@?K|J D > (???? ?%cŚ!E%??I??M$&?jfMf?`??<?9v?????'aqA????,**
,??? ??????t?%6'z**kyvbك??A?o!?CD??I?q?ɬ	t??hb-5?hyз?֋???????Sp??*rSյ?J?i}?:??|3??oF?Z??????D?V5f??i?b
???vskN??l1?o6V??n?ؽD89??????|?????$V?^????%?cF?ۚeo2??M??p??f?S??C?TsD6 `"????Ȩ??,?D|b?3b?F??? ?A?X0M@???v??`?_??"??>iؓ*?H C<????]???,<??o???7T?ں??raM?? ??t|8O`d?{????>HЃ`??``O+3    O     %  ?     <   ????2?"?VZ#Դ?S?????                      if get_option('python3')
    t     =  ?     t   ????w?S???*Dx0??e?τ`?@              ?  ?   1		install_dir : get_option('libdir') / 'gplugin'
    ?     0  ?   	  ?   	????`\4???m,<,?ht???z                  E   $	if not get_option('introspection')
