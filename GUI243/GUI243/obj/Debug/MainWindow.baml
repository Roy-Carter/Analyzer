   M S B A M L   `   `   `  ����A  =GUI243, Version=1.0.0.0, Culture=neutral, PublicKeyToken=nullclr-namespace:GUI243GUI243  5          GUI243.MainWindow   5      \ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    5      R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089:x,http://schemas.microsoft.com/winfx/2006/xaml     5      @d2http://schemas.microsoft.com/expression/blend/2008     5      &localclr-namespace:GUI243     5      b ^MaterialDesignThemes.Wpf, Version=4.0.0.2422, Culture=neutral, PublicKeyToken=df2a72020bd7962aFmaterialDesign1http://materialdesigninxaml.net/winfx/xaml/themes  5      Dmc;http://schemas.openxmlformats.org/markup-compatibility/2006  5        � Title$  Analyzer��5	      $
��1280��5
      $	��720��5         MaterialDesignPaper#���   5        MaterialDesignFont#���  5       � Icon$ images/airforce.png��5       � 
ResizeMode$ NoResize=�5        MaterialDesignBody#���  5      .��5      � .+�5   
   �� $
��1920��5      $
��1080��5      $
��Left=�5      $	��Top=�5      ��5      �� 5       �� 
StartPoint$ 	0.023,0.5&�6&    �� EndPoint$ 	0.982,0.5&�6=   .��5      �  � Offset$ 1Q�6#    � Color$ 	#FF000040��6.   � 5      $ 	#FF010121��6#   5      5      � 5    
   -   5         �Name$ 
Main_Panel��5       $��150,101,0,0q�6#    � ColumnDefinitions 5!      �� 5"      $
��266* �6#   �� 5#      $
��799* �6#   �� 5$      $	��59* �6#   .+�5'      �� -   5        	 ��Name$	 Logger��5(      $��Stretch=�5)      ���#0101DF$��2q�5+      $��Century Gothic)�5,      $��16(�5-      $
��Bold"�5.      ���	#FF04F794��. $��Disabled=�51      $��Hidden=�52      
 �� TextAlignment$
 Center=�53      �� 55      -   5        $	 textBox1��56      $��1��57      $��2��58      $��0,0,0,-1q�59      $��
33,0,50,50q�5:      $��Stretch=�5;      ���#0101DF$��2q�5=      $��Century Gothic)�5>      $��18(�5?      $
��Bold"�5@      
�������$
 Center=�5B      5E      � 5F   
   -   5        $ panelHeader��5G      $	��101��5H      $	��Top=�5I      ���	#FF000040.+�5M      �� $	��263��5N      $��62��5O      $��1011,39,0,0q�5P      $
��Left=�5Q      $	��Top=�5R      ���	#7F0505465S      '  !MaterialDesignThemes.Wpf.PackIcon  5T      $��30��5U      $��30��5V      $��	0,59,70,0q�5W      $��Right=�5X      
�������   Kind$ Refresh=�5Z      �� 5[      -   5         ��Name$ ResetButton��5\      $��59��5]      $��
0,59,11,10q�5^      $��Right=�5_      $��Reset��5a      $
��Bold"�5b      
������� J� RenderTransformOrigin$ 
0.808,0.49&�5d        MaterialDesignToolButton#��[ 5e      �� 5f      $	��104��5g      $��17��5h      $��1142,17,0,0q�5i      $
��Left=�5j      $	��Top=�5k      
�������$��@Roy Carter��5m       �� TextAlignment$ Right=�5n      $
��Wrap=�5o      A� 5p      -   5         A�Name$ ActiveCircle��5q      $��25��5r      $��25��5s      
���  ��$��	0,0,300,0q�5u      �� 5x      $��Center=�5y      $��Center=�5z      $��Century Gothic)�5{      $��30(�5|      $
��Bold"�5}      
��������� 5      $�Protocol��6    �� 5�      $
��Bold"�5�      ���#819FF7$�Analyzer��5�      5�      5�      � 5�   
   -   5        $ 	sidePanel��5�      $	��150��5�      $
��Left=�5�      ���	#FF02024F.+�5�      ?  9MaterialDesignThemes.Wpf.Transitions.TransitioningContent  -   5          Name$ TrainsitionigContentSlide��5�   2   C  =MaterialDesignThemes.Wpf.Transitions.TransitioningContentBase   OpeningEffect 6U   D  >MaterialDesignThemes.Wpf.Transitions.TransitionEffectExtension  *SlideInFromLeft+   Duration$ 0:0:0.2X��� 5�      .+�5�      �� $��1��5�      $	��130��5�      $��images\af.jpg��5�      $��UniformToFill=�5�      �� 5�      
������ 
������ $��Century Gothic)�5�      $��14(�5�      $
��Bold"�5�      
�������$��Disabled=�5�      .@�5�      �� -	   5         ��Name$ listViewItem��5�      $��80��5�      $ 0.5,0.5&�5�      z�5�      V� 5�      .~�5�   "   �� �� 5�   "   �� 5�   "   T� 5�   "   5�      .��5�      �� $
��10,0q�6%    �� Orientation$ 
Horizontal=�63   .+�5�        $��30��5�   !   $��30��5�   !   $��5q�5�   !   $��Center=�5�   !   $	 Run=�5�   !   �� 5�      -
   5        $ Start_Server��5�   !   $��30��5�   !   $��10q�5�   !   +  %MaterialDesignThemes.Wpf.RippleAssist   Feedback
 ���� $��Start��5�   !   $
��Bold"�5�   !   
�������$ 0.447,0.281&�5�   !   #��[ 5�   !   5�      5�      �� 5�      -   5        $ listViewItem2��5�      $��80��5�      $ 0.5,0.5&�5�      z�5�      V� 5�      .~�5�   "   �� �� 5�   "   �� 5�   "   T� 5�   "   5�      .��5�      �� $
��10,0q�6%   $ 
Horizontal=�63   .+�5�        $��30��5�   !   $��30��5�   !   $��5q�5�   !   $��Center=�5�   !   $ Upload=�5�   !   �� 5�      -   5        $ b1��5�   !   $��Center=�5�   !   $��Upload��5�   !   $
��Bold"�5�   !   
�������#��[ 5�   !   5�      5�      5�      5�      5�      5�      5�      