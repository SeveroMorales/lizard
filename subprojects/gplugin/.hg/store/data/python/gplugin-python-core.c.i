        x  1       /����������H͉U�Bh�W�y���<̀            (�/�`1u &�D%�hn�C�"�a�-D�hL�n� *p�M� �(9 : 7 ��}���S4)g�Z�3֢iGS�$D;�mU�)j|�}�;|{H�z�ӊ��T��:@�C��^vNw]Vl����]�k�$�d�Q�o�{��U�DZ�@�&@t�*I�H�%2E����Xb�x=�}5�^-߀��F�瓺O��J���˱�I��h��k�zW���Օ�E0���t���Pn2�Ck��B�LP�"��B�cD0E�<?�`�z�[����\��<#  `�����ĸ���&�$�.��v�w1#��ΐ�x<��x%X����0#.��z���`���A��6bo=3��	`+P��    x    �  �       0    �����kmU����$;��Ѕ���            (�/�`�= �[/p�M �����!�I����JgJ����d�-�ȵN���i)LD��L���qmP I S ��=)�[���ȢϽ��̬�V^���w��\]>�@m�4�k�VL� �xR������q� ~�0x���U��#L �Wd?���̀����$k嬮�^�5kqn��M��.�UF�^ã)r�X�:.y8���X����-����ז:/�"{�7�D�i���ip4УaX<���Ș���V�ӊ��~�	L�G�<��'��ārޓ��SR9��C��f��պ�ޖf�8bY`$d&�e�i�,�a�n�pUyEo���K��Umt������������N$|��]YR��#� y$�?M�d�@O! 0�c�5,��[�au�;+:X0����05� ڋ�.�Pg$�Vϙ��2}8�^?p6Lz�XD�@�>�whY��R.	����,    I     L        L   ����lV������j[u���qD                  >   @ * Copyright (C) 2011-2012 Gary Kramlich <grim@reaperworld.com>
    �     (        c   ����f�����c�	���p%�N              �  �   #include <gplugin-native.h>
    �    �  �      �   ������@�&S�#��\+��e�            (�/�`�E VVN6`i� 0LMA��"h�g;�hO��e�� ��J�������M�Hq�� �O��C @ 9 t��1�I�-c�|:J��Q��[�� `w�bW�b�90��g�gE�Ó���A��Ă����
vvӤU
~T�T1ώ��T��o)���,q�y$"
�(p���8 �8�Ì6)��+���ޥ��a)>�ީ�O �<E0��\�v�N�ꆿ����oB)�����O��k�1��T��h�]��bL�h$,78�W���Aj�l��Bo�X�2���5�]ϼ���"�bT��WXA~!�4
*  c����:�B.�/��C���H�����%��n^̛
�������?,�@���V~fce��6l��
2$&yB���s��֢.('Z/�2�'    o     H  �      �   ������-wD'��@�QW�**              �     <		"website", "http://bitbucket.org/rw_grim/gplugin",
		NULL
    �     X  D      �   ���������{r���oS�,�ô            (�/� p}   � d		"flags", GPLUGIN_INFO_FLAGS_LOAD_ON_QUERY |
		 INTERNAL,
 XY �*��9"         ;  R      �   �����fa���ݘN��bj�:              D  e   /	return g_object_new(GPLUGIN_TYPE_PLUGIN_INFO,
    J     u  |      �   ����!w #��E,#�	��{|��(            (�/� �e   '  D   'gplugin_query(GError **e) {
  �  �   Cload(GPNative *,   @   Eun	 ��CRP|�_�ܼ�zY����    �     m  Y      �   ������lOֽNk;d��j[�              N  �   a	return gplugin_plugin_info_new(
		"gplugin-python-loader",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
    ,     r  �   	   �   	����D?��D�a�i�����a�6:            (�/� uM PoO�JHh����S��o=F0��0^���1���m�iL���b�a<n,�#<�G�!HBW@����I�/\y6�#�hD�d�?��T�� �<X�ypY    �     $  �   
     
�����)�;�:���X�-WrכA�              �  �   		"license-id", "GPL3",
    �     d  �        ������Wr�Ɂ���ٜm��/�+              N  �   X	const gchar * const authors[] = {
		"Gary Kramlich <grim@reaperworld.com>",
		NULL
	};
    &     U  �     %   �����A�H��g���mtpk�              �  @   I		"description", "This plugin allows the loading of plugins written in "
    {     '  �     )   �����"��R�l��{v����              �  �   		"gplugin/python-loader",
    �     �  �     +   ������j���ɍU���fUF            (�/�`  �"������L-����c�X�8����w�2N�'��\���+O��7r����{
Q����ȁ�B�ôgNT:��#�����v<'YY�
`����b���ZTI�t�í�׆� mf�*�F�0�*�&�� ��A�`�
����Q�s��    	L     �  
     1   ����3�`S�@Cx)����ή��m�            (�/� ˵ B�$�W��������I�%�t0wQ� ն!߮StZ��?��~��&g���"AA� �TE�j���P1r�����*e\�ܝ/��������@v�4�f��[S��V��^���.�伲�@Q9�m�� g��M�Z��T2��f�e    	�     d  �     2   ����1-"T�c<-:�FX �+w5�)              {  �        �   L	gplugin_plugin_manager_register_loader(GPLUGIN_TYPE_PYTHON_PLUGIN_LOADER);
    
O     9  l     \   ����Y9�*0FUR�v��.��5��              V  �   -		"internal", TRUE,
		"load-on-query", TRUE,
    
�     C  n     g   ����ZL��^��ѯ�7�S�4�C�T              �  �   7		"website", GPLUGIN_WEBSITE,
		"category", "loaders",
    
�     L  n     k   �������<|:h`!�����/C                  C   @ * Copyright (C) 2011-2013 Gary Kramlich <grim@reaperworld.com>
         �  R     p   ����� �%v��ׅl|�=��-~            (�/� �� ��&��3�0��1��.�M�!�b��p��n��̙��FE������	�P�����D�v"5u�xP|*�d�G�q�k��'֨t�����c��~A�_b�$���渘[if/�f��ŭ����	 B�@*�aUjP��``����ʘ�    �     r  =     �   ����Q׻v���_�XT-��Q            (�/� �M   n  �    gplugin_query(GError **e) {
  �  ?   <load(GPNative *, �  @   >un i���%(��/S`^�)Z\P�    $     �  �     �   ����~�2z�$�^���:��            (�/�` 5 $  n  �   /gplugin_query(GPLUGIN_UNUSED GError **e) {
  �  1   Xload(GPNative *,
 
�  +   kun �  �*A���Ā��_����W�fA�    �    �  �     	   �����?u@�`=0�U�A��3�0            (�/�`�� �^`1Pg� �  (C(�*�L?�?y@R�?�S�[�p$2�Y""�l>�H�Bk?S M X ,z��!�̚��t�����s�/n�Pe@�VTU�8�l�z)��0��(�EN�	�jfy G��Ñ�8����}^_O����NT�@U���}�u��!��d-V\9�ry��I�}e��a�=���Ƶ�W���!c��r�_7��buI����͍������z�׶�o$�ɥ��h.���r���\*:̅A��HL6�_���B�
�
�����s�5��_�]|o��������+�>����x<
(Ґ��i����,�j4Ps:���3&�K�y7��OdxQ�����h�������Wl��'P�	�HB���o��fi9?D�4h6 ���x��U��,Vг����z�A��g�m�ے��-$6��
��5,�A�B����U�Mx0�����T    �     �  �        �����Kod�icK�P�:�.����            (�/� ��  �� ��E���#��6�Ă�!�dy7;U��Y
��i�Ar��<����/�$�
�k�F��Bsp�=�k챥�p*�|�Q�G�?� v���%3����ց
:���xɬ        �  �     z   ����3�Ř4o��l����X�3<            (�/�`�E w�'аl d@(uP���x��q%-8U?�V�k;O�{���� � � u��f�N���y%G��p�Ø9Ǟ��ʳ��Mj�_&���f�қ�e�Y%i�-�|��}3G�Q3�
v�pT���3��������n)^M�gn�����n���I �Fᾮb�Fr+����{��@�[J/��������2<r'R	��߸���9Θ�q�3�w��g�
�My�;w�UØ�a.֕Y� *c�����8��*lr��&jZ��
z`��M�x���9������a��o��*躠�WTб��M�aF���9����=o�G�9J��G*��wX�_�x�KNP�U��Q�Ty�L���S;��r;ǲ�p(*�G�<�_Xخ��#o�mݯ�us8�p$�ݑ��U�ƐU�&=��Cs"�:Yұ�bQDZDe�Mr���(͜�߄V�3�t��Q���o������p�JB�~��+%�E�J�`s�wpf{ɭ��yK���X"9<h���Bi�U�i������l�@��Ύ��h8w$��A%�f���͵�+��0��i3,J
w2��@PvW����<��-^S��i��}r�f��?�ۡ�%����$nN.y귞K%��:�����$m�t>�e�\��6-@Z$1a~��$3� E���   �bw�TA�#Đ�2

JJ�m1����Lnɍ�dRȤh�/܇�8��~d�EM`����)�-aeLN�:H�b��|s��ڀ)ƌ�{Ǎ 6K 7���܀�A~�����%�j��w� �x��G�91jD\2�i�XJ�p�@��&5�۠������~�r�?��s��>Wxr%8�n���GizY��욨�/1o��?i$�_�t���y]+Ԁ@j�os ����x-X8TTc���E8e&h,��6��x僙���6�����K�6රC��    �     t       �   ����L�(�E.C�å�x%���{b              �  �   h	g_set_error_literal(
		error,
		GPLUGIN_DOMAIN,
		0,
		_("The Python loader can not be unloaded")
	);

    \     )  *     �   �����)�:�Dy�G�
T���}����                   #include <glib/gi18n-lib.h>

    �     +       �   ������n��W޺�x��Y_g��              �  �                  GError **error)
    �     1  )     �   ����������	��g�����.�x�              �     %		"license-id", "LGPL-2.0-or-later",
    �     X  (        ���� ���3��0�Qd�H`��#e�                  C   @ * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
  (  )        9     (  )     8   ����_��md��`Ƈ�L�j8EX              G  b   		"gplugin/python3-loader",
    a     K  V     O   �������AL /Wg
�r�"��            (�/� P �  & 	/* clang-format off */
    %    		NULL);
n */
 �ʗ�