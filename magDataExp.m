clear;
clc;
close all;

fig = figure("Name","三軸磁場分量變化圖","Position",[100 50 1000 800]);
colors = lines(8);
% offset_gap = 0.1;



for i = 1:8
    filename = sprintf("MAG_CH%d.txt", i-1);

    data = fopen(filename, "rt");

    while ~feof(data)
        tmp  = fgetl(data);
        if contains(tmp, "PC_TIMESTAMP")
            fgetl(data);
            break;
        end
    end
    
    mag = textscan(data,'%*[^|]| %f %f %f');
    fclose(data);

    X = mag{1};
    Y = mag{2};
    Z = mag{3};
    
    % X = X + (i * offset_gap);
    % Y = Y + (i * offset_gap);
    % Z = Z + (i * offset_gap);


    subplot(3,1,1);
    hold on; grid on;
    plot(X,"Color",colors(i,:),"LineWidth",1);
    title('X 軸分量', 'FontSize', 14, 'FontWeight', 'bold');
    ylabel('X (Gauss)', 'FontSize', 12);

    subplot(3,1,2);
    hold on; grid on;
    plot(Y,"Color",colors(i,:),"LineWidth",1);
    title('Y 軸分量', 'FontSize', 14, 'FontWeight', 'bold');
    ylabel('Y (Gauss)', 'FontSize', 12);

    subplot(3,1,3);
    hold on; grid on;
    plot(Z,"Color",colors(i,:),"LineWidth",1);
    title('Z 軸分量', 'FontSize', 14, 'FontWeight', 'bold');
    ylabel('Z (Gauss)', 'FontSize', 12);
end


% legend("Location","bestoutside");
linkaxes(findall(gcf,'type','axes'), 'x');
