function [tspan, yout] = ode4(dydt, tspan, y0, options)

if nargin > 3
    v = odeget(options,'OutputFcn');
end

status = 0;

if nargin > 3 && ~isempty(v)
    status = v([tspan(1) tspan(end)],y0,'init');
end

if status == 1
    return
end

yout = zeros(length(tspan), length(y0));

y(:,1) = y0(:,1);
yout(1,:) = y(:,1);
h = (tspan(2)-tspan(1)); % wyliczam krok - zakladam ze jest staly
timePoints = length(tspan);
t = tspan;

for i = 1:1:(timePoints-1)
    s1(:,1) = dydt(t(i),y(:));
    s2(:,1) = dydt(t(i)+h/2, y(:)+h*s1(:)/2);
    s3(:,1) = dydt(t(i)+h/2, y(:)+h*s2(:)/2);
    s4(:,1) = dydt(t(i)+h, y(:)+h*s3(:));
    y(:,1) = y(:,1) + h*(s1(:,1) + 2*s2(:,1) + 2*s3(:,1) + s4(:,1))/6;
    yout(i+1, :) = y(:,1);
    if nargin > 3 && ~isempty(v)
        status = v(t(i+1),y(:),[]);
    end
    if status == 1
        yout = yout(1:i+1, :);
        tspan = tspan(1:i+1);
        return
    end
end

if nargin > 3 && ~isempty(v)    
    status = v([], [] ,'done');
end

if status == 1
    return
end

end
