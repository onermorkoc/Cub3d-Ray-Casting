/*
=> Bu klavuz kimler için:

Minilibx kütüphanesinin nasıl çalıştığını bilenler ve elbette cub3d pdf okuyanlar için :)

=> Not:

Cub3d ödevde pdf de ray castingin cok detaylı matematiğini bilmemize gerek olmadığını belirtilmiştir. Sadece nasıl çalıştığını nasıl bir aloritmaya sahip olduğunu 
arka planda neler döndüğünü bilmemiz yeterlidir. Bu klavuzdada formüllü kısımları atladığımı fark ediceksiniz çünkü o kadar bende zeki değilim .d

=> Ray Casting Nedir:

Raycasting, 2B haritada 3B perspektif oluşturmak için bir tekniktir. Raycasting basitce ekranın her dikey çizgisi için(yani her x için) 
bir hesaplama yapıyoruz.

=> Ray Casting çalışma mantığı:

Ekranın her dikey şeridi için(yani her x için), oyuncun konumundan başlayan bir ışın gönderiyoruz(\./). Bu ışın bir duvara çarpana kadar ilerlemeye devam edecek.
Eğer bir duvara çarparsa, çarptığı yerden oyuncuya olan mesafesini hesaplıyoruz (bunun için bir formül mevcut). Hesapladığımız bu mesafeyi
çizilmesi gereken duvarların yüksekliğini hesaplarken kullanıcaz. Duvar ne kadar uzaktaysa ekranda o kadar küçük gözükecek ve ne kadar yakınsa
o kadarda büyük gözükecek (dogru orantı).Oyuncunun konumu ile duvar arasındaki mesafe artarsa duvarın yüksekligi azalır (Ters orantı)

=> FOV nedir:

Oyunucunun konumundan sag ve sol iki vektör(ışın) yolluyoruz( \./ ) bu iki ışın arasındakı açıya Görüş Alanı veya FOV denir.

=> Derleme:

gcc raycasting_example.c libmlx_os.a -lXext -lX11 -lm -lz

=>Önemli bağlantılar:

https://lodev.org/cgtutor/raycasting.html  Ray casting kütüphanesi hakkında makale (buradaki yazdığım herseyi oradan baz alarak yazdım)
https://github.com/keuhdall/images_example Minilibxde boş resim oluşturma ve içindeki pixelleri nasıl doldurulduğunu gösteren klavuz

*/

# include "mlx.h"    // minilibx kütüphanesi
# include <math.h>   // matematik fonksiyonları için
# include <stdio.h>  // debug için (printf)
# include <stdlib.h> // malloc ve exit için

# define width 1500	// oyun penceresinin genişliği
# define height 850	// oyun penceresinin yüksekliği
# define img_width 64	// bastırılacak resmin genişligi
# define img_height 64	// bastırılacak resmin yüksekliği

typedef struct	s_data
{
	//posX ve posY oyuncunun konumu temsil eder
	double posX;
	double posY;

	//dirX ve dirY oyuncunun yönünü temsil eder
	double dirX;
	double dirY;

	/*
	planeX ve planeY kamera bakış açısını temsil eder.Kamera hep dik olmalıdır yani planeX hep 0 olmalı.
	planeY FOV oluyor  FOR şöyle hesaplanır FOR = 2 * atan(0.66/1.0) buradaki atan fonksiyonu tanjat hesaplayan bir fonksiyondur
	0.66 degerini ise biz veriyoruz sonuç 66 derece çıkıyor 3d oyunlarda oldukça iyi bir bakış acısı kabul ediliyor.
	*/
	double planeX;
	double planeY; 

	void	*mlx_ptr;
	void	*win_ptr;
	int	*img_data;			// Duvarlara bastırılacak resmin pixel pixel renk kodlarını tutacak olan int dizi
	void	*screen_img;			// Ekranın boyutu kadar içi boş resim;
	int	*screen_img_data;		// Ekranın boyutu kadar olan resmin pixel pixel renk kodlarını tutan int dizi(resmin içini doldurmak için)
	int	img_x;
	int	img_y;

	//bits_per_pixel, size_line, ve endian değişkenleri mlx_get_data_addr() fonksiyonun parametreleri için kullancaz
	int	bits_per_pixel; 
	int 	size_line;
	int 	endian;

	double	moveSpeed;   // ileri geri hızı (dogru orantı)
	double	rotSpeed;    // sağ sol hızı  (dogru orantı)
}				t_data;

int	worldMap[24][24] = 
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int	print_map(t_data *s_data)
{
	int	x;

	x = 0;
	while (x < width)      //ekranın width(x genişliginde) içinde dönüyoruz
	{
		/*
		ışınları hesaplamak için cameraX ihtiyacımız var CameraX kısaca ekranın mevcut x koordinatının temsil ettiği kamera düzlemindeki x koordinatıdır
		cameraX = 2 * ekranın width'ın n. sayısı / double' a çevrilmiş ekran width -1
		*/
		double cameraX = 2 * x / (double)width -1;

		// rayDirX ve rayDirY bizim ışınlarımız
		double rayDirX = s_data->dirX + s_data->planeX * cameraX;
		double rayDirY = s_data->dirY + s_data->planeY * cameraX;
		
		/*
		mapX ve mapY ışının bulunduğu konumudur ışın ilerledikçe degerini güncellicez ve hemen ardından ışının bulunduğu konum duvar mı degıl mı bakıcaz. 
		Eger duvarsa demekkı ışın duvara çarpmıştır. Peki neden ilk değeri oyuncunun konumudur çükü ışınları oyuncunun kodunumdan gönderiyoruz
		*/
		int mapX = (int)s_data->posX;
		int mapY = (int)s_data->posY;

		//sideDistX ve sideDistY ışının ilk çarptığı x ve y cizgilerin arasındaki uzaklıkğı temsil eder(raycastdelta.jpg inceleyin).Değeri ileride hesaplanılacak
		double sideDistX;
		double sideDistY;
		
		/*
		deltaDistX ve deltaDistY ise sideDist' nin devamı gibi düşünebilirz ışının x ve y cizgilerine çartıgının(yani sideDist) 
		1 sonraki x ve y cizgilerine çartığı mesafedir.(raycastdelta.jpg inceleyin) harita kare olduğundan ışınlar çapraz çapraz
		gittiğinden dolayı uzunlugu bulmak için üçgen çizilir ve pisagor ile ışının uzunlugu bulunur lakin bu sadeleşip
		fabs(1/ ışınınX_ekseni) olmuştur abs fonksiyonu mutlak değer hesaplar fabs ise double değerler için mutlak değer hesaplar.
		*/
		double deltaDistX = fabs(1 / rayDirX);
		double deltaDistY = fabs(1 / rayDirY);

		//perpWallDist oyuncunun konumdan duvara kadar olan uzaklık için kullanıcaz
		double perpWallDist;
		
		/*
		stepX ve stepY ışının ilerleme durumunun x ve y kordinatları üzerinde hangi yönde olduğunu(kordinatlar '-' li yönündede olabilir) belirtmesi için kullancaz
		değerleri hep +1 yada -1 olacak (kısaca pozitif x ve ya üzerinde ilerliyorsa ışın: degeri +1 eğer - li yönlerde ilerliyorsa +1)
		*/
		int stepX;
		int stepY;
		
		int hit = 0; //duvara çarpma olup olmadığının kontrolü için.
		int side; //Eğer gönderdiğimiz ışın x-taraflı duvara çarptıysa, side = 0, eğer bir y-taraflı duvara carptıysa, side = 1 olur.

		if (rayDirX < 0) // eğer x ışını - li yönde ise
		{
			stepX = -1; //stepX degeri -1
			sideDistX = (s_data->posX - mapX) * deltaDistX;
		}
		else
		{
			stepX = 1; // değilse + 1 dir
			sideDistX = (mapX + 1.0 - s_data->posX) * deltaDistX;
		}
		if (rayDirY < 0) //eğer y ışını - li yönde ise
		{
			stepY = -1; //stepY -1 
			sideDistY = (s_data->posY - mapY) * deltaDistY;
		}
		else
		{
			stepY = 1; //degilse +1 dir
			sideDistY = (mapY + 1.0 - s_data->posY) * deltaDistY;
		}

		while (hit == 0) // eger ışın duvara çarpmadıysa
		{
			if (sideDistX < sideDistY)
			{
				sideDistX += deltaDistX;  //duvara çarpmadıgı vakıtce ışının uzunluguna sürekli deltadist ekle
				mapX += stepX; // duvara carpmadıkca ısının konumu hep +1 veya -1 eklenıyor (ışının - li yöndede olabilir örnek x,y => (-2, -5))
				side = 0;  //ışın x-taraflı duvara çarptı
			}
			else
			{
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1; //ışın y-taraflı duvara çarptı
			}
			/*
			eger ışının bulundugu yada ilerlediği konumda diyebiliriz (bunlar bizim mapX ve mapY degerlerimizdi) haritada ki degeri 0 dan büyükse
			yani duvara çarptıysa hit = 1 olur 
			*/
			if (worldMap[mapX][mapY] > 0)
				hit = 1; // duvara çarptı degeri 1 yap döngüyü kır
		}
		if (side == 0) //burada duvar ile oyunun arasındakı uzaklıgı hesaplanıyor x taraflı duvar carptıysa ayrı y taraflı duvara carptıysa ayrı hesaplanıyor
			perpWallDist = (mapX - s_data->posX + (1 - stepX) / 2) / rayDirX;
		else
			perpWallDist = (mapY - s_data->posY + (1 - stepY) / 2) / rayDirY;

		int lineHeight = (int)(height / perpWallDist); // basılacak resmin yüksekliği
		int drawStart = -lineHeight / 2 + height / 2;
		if(drawStart < 0)
			drawStart = 0;
		int drawEnd = lineHeight / 2 + height / 2;
		if(drawEnd >= height)
			drawEnd = height - 1;

		/* Color Basma
		int	color;
		if (worldMap[mapY][mapX] == 1)
			color = 0xFF0000;
		else if (worldMap[mapY][mapX] == 2)
			color = 0x00FF00;
		else if (worldMap[mapY][mapX] == 3)
			color = 0x0000FF;
		else if (worldMap[mapY][mapX] == 4)
			color = 0xFFFFFF;
		else
			color = 0xFFFF00;
		
		if (side == 1)
			color = color / 2;

		while (drawStart <= drawEnd)
		{
			mlx_pixel_put(s_data->mlx_ptr, s_data->win_ptr, x, drawStart, color);
			drawStart++;
		}
		*/
		
		/*
		Eger color basmak istiyorsanız 243 den 267 kadar yorum satırına alın ve 270 satırıda yorum satırı yapıp yukardakı color basma yorum satırlarını
		acınız aynı sekıl tavan ve zemın ıcınde uygulayın
		*/

		int texNum = worldMap[mapX][mapY];  // duvarın yönü bu örnekte numaralar ile duvarların yönü belirlenmiş ama cub3d projesinde harfler ile belirlenmiştir

		double wallX;
		if (side == 0)
			wallX = s_data->posY + perpWallDist * rayDirY;
		else
			wallX = s_data->posX + perpWallDist * rayDirX;
		wallX -= floor(wallX);

		int texX = (int)(wallX * (double)img_width);
		if (side == 0 && rayDirX > 0)
			texX = img_width - texX - 1;
		if (side == 1 && rayDirY < 0)
			texX = img_width - texX - 1;

		double step = 1.0 * img_height / lineHeight;

		double texPos = (drawStart - height / 2 + lineHeight / 2) * step; //burada kac pixel basılacagı hesaplanılıyor
		for (int y = drawStart; y < drawEnd; y++)
		{
			int texY = (int)texPos & (img_height - 1); //eger basılacak pıxel sayısı tam sayı degılse burada and kapısına sokuluyor(maskeleme işlemi)
			texPos += step;
			int color = s_data->img_data[img_height * texY + texX];
			s_data->screen_img_data[y * width + x] = color;
		}
		x++;
	}
	mlx_put_image_to_window(s_data->mlx_ptr, s_data->win_ptr, s_data->screen_img, 0, 0); // son olarak resmin içi dolduruluyor ve ekrana basılıyor
	return (0);
}

/* Color basma için geçerli
void tavan_renk(t_data *s_data)
{
	int x = 0;
	int y = 0;
	while (x < width && y < height / 2)
	{
		mlx_pixel_put(s_data->mlx_ptr, s_data->win_ptr, x, y, 0255255);
		x++;
		if (x == width)
		{
			y++;
			x = 0;
		}
	}
}

void zemin_renk(t_data *s_data)
{
	int x = 0;
	int y = height / 2;

	while (x < width && y < height)
	{
		mlx_pixel_put(s_data->mlx_ptr, s_data->win_ptr, x, y, 192192192);
		x++;
		if (x == width)
		{
			y++;
			x = 0;
		}
	}
}
*/

void tavan_renk(t_data *s_data)
{
	
	int x = 0;
	int y;
	while (x < width)
	{
		y = 0;
		while (y < height / 2)
		{
			s_data->screen_img_data[y * width + x] = 0255255;
			y++;
		}
		x++;
	}
}

void zemin_renk(t_data *s_data)
{
	int x = 0;
	int y = height / 2;

	while (x < width && y < height)
	{
		s_data->screen_img_data[y * width + x] = 192192192;
		x++;
		if (x == width)
		{
			y++;
			x = 0;
		}
	}
}

int	key_press(int key, t_data *s_data)
{
	printf("Key code: %d\n", key);

	if (key == 65362) // ileri tuşu
	{
		if (!worldMap[(int)(s_data->posX + s_data->dirX * s_data->moveSpeed)][(int)(s_data->posY)])
			s_data->posX += s_data->dirX * s_data->moveSpeed;
		if (!worldMap[(int)(s_data->posX)][(int)(s_data->posY + s_data->dirY * s_data->moveSpeed)])
			s_data->posY += s_data->dirY * s_data->moveSpeed;
	}
	if (key == 65364) // geri tuşu
	{
		if (!worldMap[(int)(s_data->posX - s_data->dirX * s_data->moveSpeed)][(int)(s_data->posY)])
			s_data->posX -= s_data->dirX * s_data->moveSpeed;
		if (!worldMap[(int)(s_data->posX)][(int)(s_data->posY - s_data->dirY * s_data->moveSpeed)])
			s_data->posY -= s_data->dirY * s_data->moveSpeed;
	}
	if (key == 65363) //sağ
	{
		double oldDirX = s_data->dirX;
		s_data->dirX = s_data->dirX * cos(-s_data->rotSpeed) - s_data->dirY * sin(-s_data->rotSpeed);
		s_data->dirY = oldDirX * sin(-s_data->rotSpeed) + s_data->dirY * cos(-s_data->rotSpeed);
		double oldPlaneX = s_data->planeX;
		s_data->planeX = s_data->planeX * cos(-s_data->rotSpeed) - s_data->planeY * sin(-s_data->rotSpeed);
		s_data->planeY = oldPlaneX * sin(-s_data->rotSpeed) + s_data->planeY * cos(-s_data->rotSpeed);
	}
	if (key == 65361) //sol
	{
		double oldDirX = s_data->dirX;
		s_data->dirX = s_data->dirX * cos(s_data->rotSpeed) - s_data->dirY * sin(s_data->rotSpeed);
		s_data->dirY = oldDirX * sin(s_data->rotSpeed) + s_data->dirY * cos(s_data->rotSpeed);
		double oldPlaneX = s_data->planeX;
		s_data->planeX = s_data->planeX * cos(s_data->rotSpeed) - s_data->planeY * sin(s_data->rotSpeed);
		s_data->planeY = oldPlaneX * sin(s_data->rotSpeed) + s_data->planeY * cos(s_data->rotSpeed);
	}
	if (key == 65307) //esc
		exit(0);
	
	// Her haraket ettiğinde ekranı sil tekrandan tavan zemini ve güncellenmiş haritayı bas
	mlx_clear_window(s_data->mlx_ptr, s_data->win_ptr);
	tavan_renk(s_data);
	zemin_renk(s_data);
	print_map(s_data);
	return (0);
}

int	main(void)
{
	t_data s_data;

	//struct içindeki degişkenlerin açıklamaları için 50. satıra bakınız.
	s_data.posX = 12;
	s_data.posY = 5;
	s_data.dirX = -1;
	s_data.dirY = 0;
	s_data.planeX = 0;
	s_data.planeY = 0.66;
	s_data.moveSpeed = 0.05;
	s_data.rotSpeed = 0.05;
	
	s_data.mlx_ptr = mlx_init();
	s_data.win_ptr = mlx_new_window(s_data.mlx_ptr, width, height, "mlx");

	void *img_ptr = mlx_xpm_file_to_image(s_data.mlx_ptr, "./wall.xpm", &s_data.img_x, &s_data.img_y);    //resmi alıyoruz
	s_data.img_data = (int *)mlx_get_data_addr(img_ptr, &s_data.bits_per_pixel, &s_data.size_line, &s_data.endian); // resmin pixel pixek renk koduna ayrılmıs datasını alıyoruz

	s_data.screen_img = mlx_new_image(s_data.mlx_ptr, width, height); //ekranı tam kaplıyacak şekilde yeni resim oluşturuyoruz
	s_data.screen_img_data = (int *)mlx_get_data_addr(s_data.screen_img, &s_data.bits_per_pixel, &s_data.size_line, &s_data.endian); //oluşturduğumuz resmin pixellerini doldurmak için datasını alıyoruz

	//Oyun ilk kez başlayınca tavan zemini ve haritayı bas
	tavan_renk(&s_data);
	zemin_renk(&s_data);
	print_map(&s_data);
	mlx_hook(s_data.win_ptr, 2, 1L << 0, &key_press, &s_data);  // hareket kontrolü için;

	mlx_loop(s_data.mlx_ptr);
	return (0);
}